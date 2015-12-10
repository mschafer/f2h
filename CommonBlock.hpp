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
    using Handle = std::unique_ptr<CommonBlock>;
    using CommonMap = std::unordered_map<std::string, Handle>;

    void extract(const llvm::DWARFDebugInfoEntryMinimal *die, llvm::DWARFCompileUnit *cu);
    void insertPadding();

    static CommonMap map_;
    std::string name_;
    std::string linkageName_;
    std::vector<Variable::Handle> vars_;
};

#endif
