//===------------- PatternComparator.h - Code pattern finder --------------===//
//
//       SimpLL - Program simplifier for analysis of semantic difference      //
//
// This file is published under Apache 2.0 license. See LICENSE for details.
// Author: Petr Silling, psilling@redhat.com
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the LLVM code pattern finder and
/// comparator.
///
//===----------------------------------------------------------------------===//

#ifndef DIFFKEMP_SIMPLL_PATTERNCOMPARATOR_H
#define DIFFKEMP_SIMPLL_PATTERNCOMPARATOR_H

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Module.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;

// Forward declarations
class DifferentialFunctionComparator;

/// Representation of difference pattern metadata configuration.
struct PatternMetadata {
    /// Limit for the number of following basic blocks.
    int basicBlockLimit = -1;
    /// End of the previous basic block limit.
    bool basicBlockLimitEnd = false;
    /// Marker for the first differing instruction pair.
    bool firstDifference = false;
};

/// Representation of the whole difference pattern configuration.
struct PatternConfiguration {
    /// Logging option for parse failures.
    std::string onParseFailure;
    /// Vector of paths to pattern files.
    std::vector<std::string> patternFiles;
};

/// Representation of a difference pattern pair.
struct Pattern {
    /// Name of the pattern.
    const std::string name;
    /// Function corresponding to the new part of the pattern.
    const Function *newPattern;
    /// Function corresponding to the old part of the pattern.
    const Function *oldPattern;
    /// Map of all included pattern metadata.
    std::unordered_map<const Instruction *, PatternMetadata> metadataMap;
    /// Comparison start position for the new part of the pattern.
    const Instruction *newStartPosition = nullptr;
    /// Comparison start position for the old part of the pattern.
    const Instruction *oldStartPosition = nullptr;
    /// Current comparison position for the new part of the pattern.
    mutable Instruction *newPosition = nullptr;
    /// Current comparison position for the old part of the pattern.
    mutable Instruction *oldPosition = nullptr;

    Pattern(const std::string &name,
            const Function *newPattern,
            const Function *oldPattern)
            : name(name), newPattern(newPattern), oldPattern(oldPattern) {}

    bool operator==(const Pattern &Rhs) const {
        return newPattern == Rhs.newPattern && oldPattern == Rhs.oldPattern;
    }
};

// Define a hash function for difference patterns.
namespace std {
template <> struct hash<Pattern> {
    std::size_t operator()(const Pattern &pattern) const noexcept {
        return std::hash<std::string>()(pattern.name);
    }
};
} // namespace std

/// Compares difference patterns againts functions, possibly eliminating reports
/// of prior semantic differences.
class PatternComparator {
  public:
    /// Name for pattern metadata nodes.
    static const std::string MetadataName;
    /// Prefix for the new side of difference patterns.
    static const std::string NewPrefix;
    /// Prefix for the old side of difference patterns.
    static const std::string OldPrefix;
    /// Function against which new sides of patterns should be compared.
    const Function *NewFun;
    /// Function against which new sides of patterns should be compared.
    const Function *OldFun;

    PatternComparator(std::string configPath);

    ~PatternComparator();

    /// Initializes a new pattern comparison againts the given function pair.
    PatternComparator *initialize(const Function *NewFun,
                                  const Function *OldFun);

    /// Add a new difference pattern.
    void addPattern(std::string &path);

    /// Checks whether any difference patterns are loaded.
    bool hasPatterns() const;

    /// Retrives pattern metadata attached to the given instruction, returning
    /// true for valid pattern metadata nodes.
    bool getPatternMetadata(PatternMetadata &metadata,
                            const Instruction &Inst) const;

  private:
    /// Settings applied to all pattern files.
    StringMap<std::string> GlobalSettings;
    /// Set of loaded difference patterns.
    std::unordered_set<Pattern> Patterns;
    /// Map of loaded pattern modules.
    std::unordered_map<Module *, std::unique_ptr<Module>> PatternModules;
    /// Map of loaded pattern module contexts.
    std::unordered_map<Module *, std::unique_ptr<LLVMContext>> PatternContexts;

    /// Load the given configuration file.
    void loadConfig(std::string &configPath);

    /// Initializes a pattern, loading all metadata and start positions.
    bool initializePattern(Pattern &pattern);

    /// Parses a single pattern metadata operand, including all dependent
    /// operands.
    int parseMetadataOperand(PatternMetadata &patternMetadata,
                             const MDNode *instMetadata,
                             const int index) const;
};

#endif // DIFFKEMP_SIMPLL_PATTERNCOMPARATOR_H
