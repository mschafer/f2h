#include "Variable.hpp"
#include <llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h>
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
        o << "[" << d.first << ":" << d.second << "]";
    }
    o << " @ " << var.location_ << ", size " << var.size_ << " bytes \n";
    return o;
}

std::pair<llvm::dwarf::TypeKind, uint64_t>
extractBaseType(const llvm::DWARFDebugInfoEntryMinimal *dieType,
                     llvm::DWARFCompileUnit *cu)
{
    using namespace llvm;
    const uint64_t fail = static_cast<uint64_t>(-1);
    std::pair<llvm::dwarf::TypeKind, uint64_t> ret;

    auto typeTag = dieType->getTag();
    assert(typeTag == dwarf::DW_TAG_base_type || typeTag == dwarf::DW_TAG_string_type);
    ret.second = dieType->getAttributeValueAsUnsignedConstant(cu, dwarf::DW_AT_byte_size, fail);
    if (typeTag == dwarf::DW_TAG_base_type) {
        auto tmp = dieType->getAttributeValueAsUnsignedConstant(cu, dwarf::DW_AT_encoding, fail);
        assert(tmp != fail);
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

Variable::Variable()
{
    
}

Variable::~Variable()
{
    
}

std::unique_ptr<Variable>
Variable::extract(const llvm::DWARFDebugInfoEntryMinimal *die,
                  llvm::DWARFCompileUnit *cu)
{
    using namespace llvm;
    const uint64_t fail = static_cast<uint64_t>(-1);

    std::unique_ptr<Variable> r(new Variable());
    r->name_ = die->getName(cu, DINameKind::ShortName);
    
    DWARFFormValue locForm;
    bool t = die->getAttributeValue(cu, dwarf::DW_AT_location, locForm);
    if (t) {
        // formal parameters use data4 instead of block (getAsSignedConstant?)
        auto locBlock = locForm.getAsBlock();
        if (!locBlock.hasValue()) {
            auto locConstant = locForm.getAsSignedConstant();
            assert(locConstant.hasValue());
            r->location_ = locConstant.getValue();
        } else {
            auto val = locBlock.getValue();
            if (val[0] == dwarf::DW_OP_addr) {
                uint64_t addr;
                unsigned char *paddr = reinterpret_cast<unsigned char *>(& addr);
                std::copy(val.begin()+1, val.begin()+9, paddr);
                r->location_ = addr;
            } else {
                // no address so this is a register or stack location which means the variable must
                // be a subroutine parameter and we don't need the location
                r->location_ = static_cast<uint64_t>(-1);
            }
        }
    }

    // type
    auto typeOffset = die->getAttributeValueAsReference(cu, dwarf::DW_AT_type, fail);
    auto dieType = cu->getDIEForOffset(typeOffset);
    auto typeTag = dieType->getTag();

    switch (typeTag) {
        case dwarf::DW_TAG_base_type:
        case dwarf::DW_TAG_string_type:
        {
            auto bt = extractBaseType(dieType, cu);
            r->type_ = bt.first;
            r->size_ = bt.second;
        }
        break;
            
        case dwarf::DW_TAG_array_type:
        {
            // find the base type for the array
            auto arrayTypeOffset = dieType->getAttributeValueAsReference(cu, dwarf::DW_AT_type, fail);
            auto dieArrayType = cu->getDIEForOffset(arrayTypeOffset);
            auto bt = extractBaseType(dieArrayType, cu);
            r->type_ = bt.first;
            
            // dimensions
            auto dim = dieType->getFirstChild();
            size_t count = 1;
            while (dim && !dim->isNULL()) {
                Dimension d(0, -1);
                assert(dim->getTag() == dwarf::DW_TAG_subrange_type);
                DWARFFormValue form;
                bool t = dim->getAttributeValue(cu, dwarf::DW_AT_upper_bound, form);
                if (t) {
                    auto ub = form.getAsSignedConstant();
                    if (ub.hasValue()) d.second = ub.getValue();
                }
                
                t = dim->getAttributeValue(cu, dwarf::DW_AT_lower_bound, form);
                if (t) {
                    auto lb = form.getAsSignedConstant();
                    if (lb.hasValue()) d.first = lb.getValue();
                }
                r->dims_.push_back(d);
                count *= (d.second-d.first);
                dim = dim->getSibling();
            }
            r->size_ = bt.second * count;
        }
        break;

        case dwarf::DW_TAG_const_type:
        {
            // find the base type for the constant
            auto conTypeOffset = dieType->getAttributeValueAsReference(cu, dwarf::DW_AT_type, fail);
            auto dieConType = cu->getDIEForOffset(conTypeOffset);
            auto bt = extractBaseType(dieConType, cu);
            r->type_ = bt.first;
            r->size_ = bt.second;
        }
        break;
            
        case dwarf::DW_TAG_structure_type:
            throw std::invalid_argument("structures not supported yet");
            break;
            
        case dwarf::DW_TAG_pointer_type:
            // argv
            throw std::invalid_argument("pointers not supported yet");
            break;
            
        default:
            throw std::invalid_argument("type tag not recognized");
            break;
    }

    return r;
}

std::string Variable::cTypeDecl(llvm::dwarf::TypeKind type, size_t elementSize)
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
                    o << "float complex";
                    break;
                    
                case 16:
                    o << "double complex";
                    break;
                    
                case 32:
                    o << "long double complex";
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
    
    // element type declaration
    auto es = elementSize();
    o << cTypeDecl(type_, es) << " ";
    o << name_;
    
    // dimensions in reverse order for C
    auto itdim = dims_.rbegin();
    while (itdim != dims_.rend()) {
        auto &d = *itdim;
        o << "[" << d.second - d.first << "]";
        ++itdim;
    }
    
    // dimension of char arrays
    if (es != 1 && (type_ == dwarf::DW_ATE_signed_char || type_ == dwarf::DW_ATE_unsigned_char)) {
        o << "[" << elementSize() << "]";
    }
    
    o << ";\n";
    
    return o.str();
}
