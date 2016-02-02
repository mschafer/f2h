#include "Subprogram.hpp"
#include "CommonBlock.hpp"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugAbbrev.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h"
#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"
#include <sstream>

using namespace llvm;

Subprogram::Subprogram() : unsupported_(true)
{
    
}

Subprogram::~Subprogram()
{
    
}

Subprogram::Handle Subprogram::extract(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu)
{
    Handle r(new Subprogram());

    r->name_ = die->getName(cu, DINameKind::ShortName);
    r->linkageName_ = die->getName(cu, DINameKind::LinkageName);

    // ignore main
    if (!r->name_.compare("main")) {
        r->unsupported_ = true;
        return r;
    }
    
    auto child = die->getFirstChild();
    while (child && !child->isNULL()) {
        auto tag = child->getTag();
        if (tag == dwarf::DW_TAG_common_block) {
            try {
                CommonBlock::extractAndAdd(child, cu);
            } catch (std::runtime_error &ex) {
                errs() << "skipping common block in " << r->name_  << " because " << ex.what();
            }
        }
        
        else if (tag == dwarf::DW_TAG_formal_parameter) {
            try {
                Variable::Handle h = Variable::extract(Variable::PARAMETER, child, cu);
                
                // if there is a parameter named __result, then it is an out parameter for
                // the return value of a function.  I still haven't figured out how this works
                // so best to skip this function
                if (!h->name_.compare("__result")) {
                    errs() << "function " << r->name_ << " appears to return an array or string which is not supported yet, skipping...";
                    break;
                }
                r->args_.push_back(std::move(h));

            } catch (std::runtime_error &ex) {
                errs() << "extract subrountine " << r->name_ << " failed on parameter: " <<  ex.what();
                throw ex;
            }
        }
        
        // need to check the local variables because that is the only way to identify
        // a function that returns a value
        else if (tag == dwarf::DW_TAG_variable) {
            r->extractReturn(child, cu);
        }
        
        child = child->getSibling();
    }
    
    r->unsupported_ = false;
    return r;
}

std::string Subprogram::cDeclaration() const
{
    std::stringstream ss;

    if (unsupported_) {
        ss << "// function " << name_ << "is not supported yet\n";
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
            }
            ss << arg->cDeclaration();
        }
        
        ss << " );";
    }

    return ss.str();
}

void Subprogram::extractReturn(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu)
{
    std::string resultName = "__result_" + name_;
    const char *varName = die->getName(cu, DINameKind::ShortName);
    if (varName && !resultName.compare(varName)) {
        returnVal_ = Variable::extract(Variable::PARAMETER, die, cu);
    }
}