#include "Subprogram.hpp"
#include "CommonBlock.hpp"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugAbbrev.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h"
#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include <sstream>

using namespace llvm;

Subprogram::Subprogram() : unsupported_(true)
{
    
}

Subprogram::~Subprogram()
{
    
}

Subprogram::Handle Subprogram::extract(llvm::DWARFDie die)
{
    Handle r(new Subprogram());

    r->name_ = die.getName(DINameKind::ShortName);
    r->linkageName_ = die.getName(DINameKind::LinkageName);

    // ignore main
    if (!r->name_.compare("main")) {
        r->unsupported_ = true;
        return r;
    }
    
    // ignore subprograms with an abstract origin as they just refer to a concrete instance
    // which will be picked up later
    auto abstractOrigin = die.find(dwarf::DW_AT_abstract_origin);
    if (abstractOrigin.hasValue()) {
        r.reset();
        return r;
    }
        
    int stringParamCount = 0;
    auto child = die.getFirstChild();
    while (child.isValid() && !child.isNULL()) {
        auto tag = child.getTag();
        if (tag == dwarf::DW_TAG_common_block) {
            try {
                CommonBlock::extractAndAdd(child);
            } catch (std::runtime_error &ex) {
                errs() << "skipping common block in " << r->name_  << " because " << ex.what() << "\n";
            }
        }
        
        else if (tag == dwarf::DW_TAG_formal_parameter) {
            try {
                Variable::Handle h = Variable::extract(Variable::PARAMETER, child);
                
                // if there is a parameter named __result, then it is an out parameter for
                // the return value of a function.  I still haven't figured out how this works
                // so best to skip this function
                if (!h->name_.compare("__result")) {
                    errs() << "function " << r->name_ << " appears to return an array or string which is not supported yet, skipping...\n";
                    break;
                }
                
                // if this argument is a string, then there will be a hidden argument at the end for its length
                if (h->isString()) {
                    ++stringParamCount;
                }
                
                r->args_.push_back(std::move(h));
                

            } catch (std::runtime_error &ex) {
                errs() << "extract subrountine " << r->name_ << " failed on parameter: " <<  ex.what() << "\n";
                throw ex;
            }
        }
        
        // need to check the local variables because that is the only way to identify
        // a function that returns a value
        else if (tag == dwarf::DW_TAG_variable) {
            r->extractReturn(child);
        }
        
        child = child.getSibling();
    }
    
    // mark parameters at the end of the argument list as string lengths
    auto rit = r->args_.rbegin();
    while (stringParamCount > 0) {
        rit->get()->context_ = Variable::STRING_LEN_PARAMETER;
        --stringParamCount;
        ++rit;
    }
    
    r->unsupported_ = false;
    return r;
}

std::string Subprogram::cDeclaration() const
{
    std::stringstream ss;
    int line = 1;

    if (unsupported_) {
        ss << "// function " << name_ << " is not supported yet\n";
    } else {
        
        // return type
        if (returnVal_) {
            ss << returnVal_->cType() << " ";
        } else {
            ss << "void ";
        }
        
        // name
        ss << linkageName_ << "( ";
        
        // arguments
        size_t narg = args_.size();
        for (size_t i=0; i<narg; ++i) {
            auto &arg = args_[i];
            if (i>0) {
                ss << ", ";
                
                if (ss.tellp() > line * 100) {
                    ss << std::endl << "    ";
                    ++line;
                }
            }
            try {
                ss << arg->cDeclaration();
            } catch (std::runtime_error &ex) {
                errs() << "Subprogram::cDeclaration--argument cDecl failed: " << ex.what() << "\n";
                errs() << "Skipping " << name_ << "\n";
                throw ex;
            }
        }
        
        ss << " );";
    }

    return ss.str();
}

void Subprogram::extractReturn(llvm::DWARFDie die)
{
    std::string resultName = "__result_" + name_;
    const char *varName = die.getName(DINameKind::ShortName);
    if (varName && !resultName.compare(varName)) {
        returnVal_ = Variable::extract(Variable::PARAMETER, die);
    }
}
