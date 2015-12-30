#include "Subprogram.hpp"
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

    
    return r;
}

std::string Subprogram::cDeclaration() const
{
    std::stringstream ss;


    return ss.str();
}