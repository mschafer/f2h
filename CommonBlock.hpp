#ifndef CommonBlock_hpp
#define CommonBlock_hpp

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Variable.hpp"

namespace llvm {
class DWARFDebugInfoEntryMinimal;
class DWARFCompileUnit;

}

class CommonBlock
{
public:
    using Handle = std::shared_ptr<CommonBlock>;
    using CommonMap = std::unordered_map<std::string, Handle>;
    
    /**
     * Extracts the common common block definition from the dwarf data
     * and stores it in the map.  If the map already contains a common
     * block with the same name, then this method assumes they are
     * identical and does nothing.
     */
    static Handle extractAndAdd(llvm::DWARFDie die);

    /// Contains all common blocks added so far and indexed by name.
    static CommonMap map_;

    /// \return C declaration for this common block.
    std::string cDeclaration() const;

private:
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &, const CommonBlock &);
    static Handle extract(llvm::DWARFDie die);

    void insertPadding();

    std::string name_;
    std::string linkageName_;
    std::vector<Variable::Handle> vars_;
};

#endif
