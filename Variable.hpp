#ifndef Variable_hpp_
#define Variable_hpp_

#include <string>
#include <utility>
#include <vector>
#include <llvm/Support/Dwarf.h>
#include "llvm/Support/raw_ostream.h"

namespace llvm {
    class DWARFDebugInfoEntryMinimal;
    class DWARFCompileUnit;
}

class Variable
{
public:

    /// lower bound, upper bound
    using Dimension = std::pair<ptrdiff_t, ptrdiff_t>;
    
    Variable();
    virtual ~Variable();
    
    static std::unique_ptr<Variable> extract(
        const llvm::DWARFDebugInfoEntryMinimal *die,
        llvm::DWARFCompileUnit *cu);
    
    llvm::dwarf::TypeKind type_;
    uint64_t size_;
    uint64_t location_;
    std::string name_;
    std::vector<Dimension> dims_;
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &o, const Variable &var);

#endif
