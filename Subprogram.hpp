#ifndef Subprogram_hpp
#define Subprogram_hpp

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Variable.hpp"

namespace llvm {
class DWARFDebugInfoEntryMinimal;
class DWARFCompileUnit;

}

class Subprogram
{
public:
    using Handle = std::shared_ptr<Subprogram>;
    
    Subprogram();
    ~Subprogram();
    
    /**
     * Extract information about the subprogram from dwarf data.
     * Any referenced common blocks are added to the static map.
     */
    static Handle extract(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu);

    std::string cDeclaration() const;
    
    std::string name_;
    std::string linkageName_;
    std::vector<Variable::Handle> args_;
    Variable::Handle returnVal_;
    bool unsupported_;
    
    void extractReturn(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu);
};

#endif
