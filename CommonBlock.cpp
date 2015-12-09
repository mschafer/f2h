#include "CommonBlock.hpp"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugAbbrev.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h"
#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"

using namespace llvm;

CommonBlock::CommonMap CommonBlock::map_;

void CommonBlock::extract(const DWARFDebugInfoEntryMinimal *die, DWARFCompileUnit *cu)
{
    if (die->getTag() != dwarf::DW_TAG_common_block) {
        throw std::runtime_error("DIE is not a common block");
    }

    // names
    name_ = die->getName(cu, DINameKind::ShortName);
    linkageName_ = die->getName(cu, DINameKind::LinkageName);
    
    auto debugInfoData = cu->getDebugInfoExtractor();
    auto offset = die->getOffset();
    assert(debugInfoData.isValidOffset(offset));
    auto abbrCode = debugInfoData.getULEB128(&offset);
    assert(abbrCode);
    
    // children of common are the variables it contains
    auto child = die->getFirstChild();
    while (child && !child->isNULL()) {
        auto var = Variable::extract(child, cu);
        outs() << *var;
        outs() << var->cDeclaration();
        vars_.push_back(std::move(var));
        child = child->getSibling();
    }
}
