#include "Variable.hpp"
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFFormValue.h>
#include <type_traits>
#include <sstream>

llvm::raw_ostream &operator<<(llvm::raw_ostream &o, const Variable &var)
{
    using namespace llvm;
    
    switch (var.type_) {
        case dwarf::DW_ATE_signed:
            o << "signed ";
            break;
            
        case dwarf::DW_ATE_unsigned:
            o << "unsigned ";
            break;
            
        case dwarf::DW_ATE_float:
            o << "float ";
            break;
            
        case dwarf::DW_ATE_complex_float:
            o << "complex_float ";
            break;
            
        case dwarf::DW_ATE_boolean:
            o << "boolean ";
            break;
            
        case dwarf::DW_ATE_signed_char:
            o << "signed char ";
            break;
            
        case dwarf::DW_ATE_unsigned_char:
            o << "signed char ";
            break;
            
        default:
            o << var.type_ << " ? ";
    }
    
    o << var.name_;
    for (auto d : var.dims_) {
        o << "[" << d.getValue().first << ":" << d.getValue().second << "]";
    }
    o << " @ " << var.location_ << ", element size " << var.elementSize_ << "\n";
    return o;
}

std::pair<llvm::dwarf::TypeKind, uint64_t>
extractBaseType(llvm::DWARFDie dieType,
                     llvm::DWARFCompileUnit *cu)
{
    using namespace llvm;
    const uint64_t fail = static_cast<uint64_t>(-1);
    std::pair<llvm::dwarf::TypeKind, uint64_t> ret;
    
    // string type: then byte size has two possible meanings
    // if it is the only attribute, then it is the length of the string
    // if there is also a string length attribute, then it is the byte size of the string length

    auto typeTag = dieType.getTag();
    assert(typeTag == dwarf::DW_TAG_base_type || typeTag == dwarf::DW_TAG_string_type);
    
    auto size = dieType.find(dwarf::DW_AT_byte_size);
    if (size.hasValue()) {
        ret.second = size.getValue().getAsUnsignedConstant().getValueOr(fail);
    }
    else {
        ret.second = fail;
    }
    
    if (typeTag == dwarf::DW_TAG_base_type) {
        auto tmp = dieType.find(dwarf::DW_AT_encoding).getValue().getAsUnsignedConstant().getValue();
        ret.first = static_cast<llvm::dwarf::TypeKind>(tmp);
    } else {
        // if string_type then no encoding so use char
        if (std::is_signed<char>::value) {
            ret.first = dwarf::DW_ATE_signed_char;
        } else {
            ret.first = dwarf::DW_ATE_unsigned_char;
        }
    }
    return ret;
}

Variable::Variable() : isConst_(false)
{
    
}

Variable::~Variable()
{
    
}

std::unique_ptr<Variable>
Variable::extract(Context context, llvm::DWARFDie die)
{
    using namespace llvm;

    std::unique_ptr<Variable> r(new Variable());
    r->context_ = context;
    r->name_ = die.getName(DINameKind::ShortName);
    
    // location is only used for common block members to determine padding
    if (context == COMMON_BLOCK_MEMBER) {
        r->extractLocation(die);
    }
    
    // type information
    r->extractType(die);

    return r;
}

std::string Variable::dwarfToCType(llvm::dwarf::TypeKind type, size_t elementSize)
{
    using namespace llvm;
    std::ostringstream o;
    
    // element type declaration
    switch (type) {
        case dwarf::DW_ATE_boolean:
        case dwarf::DW_ATE_signed:
            o << "int" << elementSize*8 << "_t";
            break;
            
        case dwarf::DW_ATE_unsigned:
            o << "uint" << elementSize*8 << "_t";
            break;
            
        case dwarf::DW_ATE_float:
        {
            switch (elementSize) {
                case 4:
                    o << "float";
                    break;
                    
                case 8:
                    o << "double";
                    break;
                    
                case 16:
                    o << "long double";
                    break;
                    
                default:
                    throw std::invalid_argument("impossible float size");
            }
            break;
        }
            
        case dwarf::DW_ATE_complex_float:
        {
            switch (elementSize) {
                case 8:
                    o << "float_complex";
                    break;
                    
                case 16:
                    o << "double_complex";
                    break;
                    
                case 32:
                    o << "long_double_complex";
                    break;
                    
                default:
                    throw std::invalid_argument("impossible complex size");
            }
            break;
        }
            
        case dwarf::DW_ATE_signed_char:
        case dwarf::DW_ATE_unsigned_char:
            o << "char";
            break;
            
        default:
            throw std::invalid_argument("unknown type");
    }
    return o.str();
}

std::string Variable::cDeclaration() const
{
    using namespace llvm;
    std::ostringstream o;
    
    switch (context_) {
    
        case STRING_LEN_PARAMETER:
            o << cType() << " " << name_;
            break;
    
    /**
     * \todo we usually can't find values for dimensions of an array parameter because they
     * are passed in as other arguements or in a common block.  However, we do know how many
     * dimensions there are.  Maybe generate a comment?
     */
        case PARAMETER:
            o << cType() << " *" << name_;
            break;
    
        case COMMON_BLOCK_MEMBER:
            o << cType() << " " << name_;
            
            // dimensions
            // array dimensions
            auto itdim = dims_.rbegin();
            while (itdim != dims_.rend()) {
                if (!itdim->hasValue()) {
                    throw std::runtime_error("Variable::cDeclaration--array in common block with unspecified dimensions");
                } else {
                    auto &d = itdim->getValue();
                    o << "[" << d.second - d.first + 1 << "]";
                }
                ++itdim;
            }

            if (isString()) {
                o << "[" << elementSize_ << "]";
            }
            
            break;
    }
    return o.str();
}

void Variable::extractLocation(llvm::DWARFDie die)
{
    using namespace llvm;
    
    auto locDie = die.find(dwarf::DW_AT_location);
    if (!locDie.hasValue()) {
        throw std::runtime_error("Variable::extractLocation--no location attribute");
    }

    auto locBlock = locDie.getValue().getAsBlock();
    if (!locBlock.hasValue()) {
        throw std::runtime_error("Variable::extractLocation--not in block form");
    }
    
    auto val = locBlock.getValue();
    if (val[0] != dwarf::DW_OP_addr) {
        throw std::runtime_error("Variable::extractLocation--not an absolute address");
    }
    uint64_t addr;
    unsigned char *paddr = reinterpret_cast<unsigned char *>(& addr);
    std::copy(val.begin()+1, val.begin()+9, paddr);
    location_ = addr;
}

void Variable::extractType(llvm::DWARFDie die)
{
    using namespace llvm;
    
    const uint64_t fail = static_cast<uint64_t>(-1);
    auto typeDie = die.getAttributeValueAsReferencedDie(dwarf::DW_AT_type);
    if (!typeDie.isValid()) {
        throw std::runtime_error("Variable::extractType--no type attribute");
    }
    auto typeTag = typeDie.getTag();
    
    // arrays and const have the base type information nested one level lower in the tree
    if (typeTag == dwarf::DW_TAG_array_type) {
        extractArrayDims(typeDie);
        
        // use the type die from the array to get information about individual elements
        typeDie = die.getAttributeValueAsReferencedDie(dwarf::DW_AT_type);
        typeTag = typeDie.getTag();
    } 
    
    else if (typeTag == dwarf::DW_TAG_const_type) {
        isConst_ = true;
        
        // use the type die from the array to get information about the type of the constant
        typeDie = die.getAttributeValueAsReferencedDie(dwarf::DW_AT_type);
        typeTag = typeDie.getTag();
    }
    
    switch (typeTag) {
        case dwarf::DW_TAG_base_type:
        {
            auto tmp = typeDie.find(dwarf::DW_AT_encoding);
            if(!tmp.hasValue()) {
                throw std::runtime_error("Variable::extractType--no encoding");
            }
            type_ = static_cast<llvm::dwarf::TypeKind>(tmp.getValue().getAsUnsignedConstant().getValueOr(fail));
            
            tmp = typeDie.find(dwarf::DW_AT_byte_size);
            if(!tmp.hasValue()) {
                throw std::runtime_error("Variable::extractType--no byte size");
            }
            elementSize_ = tmp.getValue().getAsUnsignedConstant().getValue();
        }
            break;
            
        case dwarf::DW_TAG_string_type:
        {
            if (std::is_signed<char>::value) {
                type_ = dwarf::DW_ATE_signed_char;
            } else {
                type_ = dwarf::DW_ATE_unsigned_char;
            }
            
            // We only care about the string length if this is a common block member for padding determination.
            // If this is a parameter, then the length is passed as a hidden argument.
            if (context_ == COMMON_BLOCK_MEMBER) {
                // string type: then byte size has two possible meanings
                // if it is the only attribute, then it is the length of the string
                // if there is also a string length attribute, then it is the byte size of the string length
                ///\todo check to make sure there is no DW_AT_string_length attribute
                auto tmp = typeDie.find(dwarf::DW_AT_byte_size);
                if (!tmp.hasValue()) {
                    throw std::runtime_error("Variable::extractType--no byte size for string");
                }
                else {
                    elementSize_ = tmp.getValue().getAsUnsignedConstant().getValue();
                }
            } else {
                elementSize_ = fail;
            }
        }
            break;
            
        case dwarf::DW_TAG_array_type:
        {
            throw std::runtime_error("Variable::extractType--nested array types");
        }
            break;
            
        case dwarf::DW_TAG_const_type:
        {
            throw std::runtime_error("Variable::extractType--nested const types");
        }
            break;
            
        case dwarf::DW_TAG_structure_type:
            throw std::runtime_error("Variable::extractType--structures not supported yet");
            break;
            
        case dwarf::DW_TAG_pointer_type:
            // argv
            throw std::runtime_error("Variable::extractType--pointers not supported yet");
            break;
            
        default:
            throw std::runtime_error("Variable::extractType--type tag not recognized");
            break;
    }
}

void Variable::extractArrayDims(llvm::DWARFDie die)
{
    using namespace llvm;
    
    // dimensions
    auto dim = die.getFirstChild();
    while (dim.isValid() && !dim.isNULL()) {
        Dimension d(std::make_pair(1, -1));
        auto &dval = d.getValue();
        if (dim.getTag() != dwarf::DW_TAG_subrange_type) {
            throw std::runtime_error("Variable::extractArrayDims--child is not a subrange");
        }
        auto dimAttr = dim.find(dwarf::DW_AT_upper_bound);
        if (dimAttr.hasValue() && dimAttr.getValue().getAsSignedConstant().hasValue()) {
            dval.second = dimAttr.getValue().getAsSignedConstant().getValue();
        }
        else {
            d.reset();
            dims_.push_back(d);
            dim = dim.getSibling();
            continue;
        }
    
        // if no lower bound, use default of 1 for FORTRAN
        dimAttr = dim.find(dwarf::DW_AT_lower_bound);
        if (dimAttr.hasValue()) {
            auto lb = dimAttr.getValue().getAsSignedConstant();
            if (lb.hasValue()) dval.first = lb.getValue();
        }
        dims_.push_back(d);
        dim = dim.getSibling();
    }
}

size_t Variable::elementCount() const
{
    size_t r = 1;
    for (auto &d : dims_) {
        if (!d.hasValue()) {
            throw std::runtime_error("Variable::elementCount--undefined dimension");
        }
        auto &dval = d.getValue();
        r *= dval.second - dval.first + 1;
    }
    return r;
}
