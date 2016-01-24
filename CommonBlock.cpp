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

llvm::raw_ostream &operator<<(llvm::raw_ostream &o, const CommonBlock &cb)
{
    o << "common block " << cb.name_ << '\n';
    for (auto &v : cb.vars_)
    {
        o << "    " << v->cDeclaration();
    }
    return o;
}

CommonBlock::CommonMap CommonBlock::map_;

CommonBlock::Handle
CommonBlock::extractAndAdd(const DWARFDebugInfoEntryMinimal *die, DWARFCompileUnit *cu)
{
    std::string commonName = die->getName(cu, llvm::DINameKind::ShortName);
    auto fit = CommonBlock::map_.find(commonName);
    if (fit == CommonBlock::map_.end()) {
        CommonBlock::Handle cb(CommonBlock::extract(die, cu));
        CommonBlock::map_.insert(std::make_pair(commonName, cb));
        return cb;
    } else {
        return fit->second;
    }
}


CommonBlock::Handle CommonBlock::extract(const DWARFDebugInfoEntryMinimal *die, DWARFCompileUnit *cu)
{
    CommonBlock::Handle r(new CommonBlock());
    
    if (die->getTag() != dwarf::DW_TAG_common_block) {
        throw std::runtime_error("DIE is not a common block");
    }

    // names
    r->name_ = die->getName(cu, DINameKind::ShortName);
    r->linkageName_ = die->getName(cu, DINameKind::LinkageName);
    
    auto debugInfoData = cu->getDebugInfoExtractor();
    auto offset = die->getOffset();
    assert(debugInfoData.isValidOffset(offset));
    auto abbrCode = debugInfoData.getULEB128(&offset);
    assert(abbrCode);
    
    // children of common are the variables it contains
    auto child = die->getFirstChild();
    while (child && !child->isNULL()) {
        auto var = Variable::extract(Variable::COMMON_BLOCK_MEMBER, child, cu);
        r->vars_.push_back(std::move(var));
        child = child->getSibling();
    }
    r->insertPadding();
    
    return r;
}

void CommonBlock::insertPadding()
{
    assert(vars_[0]->location_ == 0);
    size_t loc = 0, padCount=1;
    auto it = vars_.begin();
    loc += (*it)->elementSize() * (*it)->elementCount();
    ++it;
    while (it != vars_.end()) {
        ptrdiff_t pad = (*it)->location_ - loc;
        assert(pad >= 0);
        if (pad) {
            std::stringstream ss;
            ss << "pad" << padCount++;
            Variable::Handle padVar(new Variable());
            padVar->location_ = loc;
            padVar->elementSize_ = 1;
            padVar->elementCount_ = pad;
            padVar->type_ = dwarf::DW_ATE_unsigned;
            padVar->name_ = ss.str();
            if (pad > 1) {
                padVar->dims_.push_back(Variable::Dimension(0, pad));
            }
            it = vars_.insert(it, std::move(padVar));
            loc += pad;
        }
        ++it;
    }
}

std::string CommonBlock::cDeclaration() const
{
    std::stringstream ss;
    ss << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n";
    ss << "extern struct { \n";
    
    for (auto &v : vars_)
    {
        ss << "    " << v->cDeclaration();
    }
    
    ss << "} " << linkageName_ << ";\n";
    ss << "#ifdef __cplusplus\n}\n#endif\n";
    
    return ss.str();
}
