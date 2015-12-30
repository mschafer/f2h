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
    using Handle = std::unique_ptr<Subprogram>;
    
    Subprogram();
    ~Subprogram();
    
    static Handle extract(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu);

    std::string cDeclaration() const;
    
    std::string name_;
    std::string linkageName_;
    std::vector<Variable::Handle> params_;
};

#endif
