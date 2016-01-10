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

Subprogram::Subprogram()
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
        r.reset();
        return r;
    }
    
    auto child = die->getFirstChild();
    while (child && !child->isNULL()) {
        auto tag = child->getTag();
        if (tag == dwarf::DW_TAG_common_block) {
            CommonBlock::extractAndAdd(child, cu);
        }
        
        else if (tag == dwarf::DW_TAG_formal_parameter) {
            Variable::Handle h = Variable::extract(child, cu);

            // if there is a parameter named __result, then it is an out parameter for
            // the return value of a function.  I still haven't figured out how this works
            // so best to error here
            if (!h->name_.compare("__result")) {
                throw std::runtime_error("functions returning a non-scalar are not supported yet");
            }
            outs() << "    " << *h;
        }
        
        // need to check the local variables because that is the only way to identify
        // a function that returns a value
        else if (tag == dwarf::DW_TAG_variable) {
            r->extractReturn(child, cu);
        }
        
        child = child->getSibling();
    }

    return r;
}

std::string Subprogram::cDeclaration() const
{
    std::stringstream ss;


    return ss.str();
}

void Subprogram::extractReturn(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu)
{
    std::string resultName = "__result_" + name_;
    const char *varName = die->getName(cu, DINameKind::ShortName);
    if (!resultName.compare(varName)) {
        Variable::Handle ret = Variable::extract(die, cu);
        outs() << name_ << " return is: " << ret->cDeclaration();
    } else {
        outs() << " no return for " << name_;
    }
}