#ifndef Variable_hpp_
#define Variable_hpp_

#include <string>
#include <utility>
#include <vector>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/ADT/Optional.h>
#include "llvm/Support/raw_ostream.h"

namespace llvm {
    class DWARFDebugInfoEntryMinimal;
    class DWARFCompileUnit;
}

class Variable
{
public:

    /// lower bound, upper bound
    using Dimension = llvm::Optional<std::pair<ptrdiff_t, ptrdiff_t> >;
    using Handle = std::unique_ptr<Variable>;
    
    enum Context {
        PARAMETER,
        STRING_LEN_PARAMETER,
        COMMON_BLOCK_MEMBER
    };
    
    Variable();
    virtual ~Variable();
    
    std::string cDeclaration() const;
    std::string cType() const {
        return dwarfToCType(type_, elementSize());
    }
    
    size_t elementCount() const;
    
    size_t elementSize() const { return elementSize_; }
    
    static std::string dwarfToCType(llvm::dwarf::TypeKind, size_t elementSize);
    
    static Handle extract(Context context, llvm::DWARFDie die);
    
    /**
     * All common block members should have the location attribute stored as a
     * block1 with the first byte being the op-code for an absolute address followed
     * by an 8 byte address.
     * Local variables and parameters will have a location attribute with a different
     * form and cause this routine to throw an exception.  Location is only needed for
     * common block members to determine padding.
     */
    void extractLocation(llvm::DWARFDie die);
    
    void extractType(llvm::DWARFDie die);
    
    void extractArrayDims(llvm::DWARFDie die);
    
    bool isString() const { return (type_ == llvm::dwarf::DW_ATE_signed_char ||
        type_ == llvm::dwarf::DW_ATE_unsigned_char); }
    
    Context context_;
    llvm::dwarf::TypeKind type_;
    uint64_t elementSize_;
    uint64_t location_;
    std::string name_;
    std::vector<Dimension> dims_;
    bool isConst_;
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &o, const Variable &var);

#endif
