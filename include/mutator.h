#ifndef MUTATOR_H
#define MUTATOR_H

#include <functional>
#include <random>
#include <string>
#include <vector>

class Mutator {
private:
    char const* source;
    std::string path;

    using MutationStrategy = std::function<std::string(std::string const&)>;
    std::vector<MutationStrategy> strategies;

    // Random generation utilities
    std::random_device rd;
    std::mt19937 gen;

    // Helper functions
    bool overwriteFile(std::string const& content);
    void initializeStrategies();
    MutationStrategy selectRandomStrategy();
    std::string randomGibberish(int length = 5);
    std::string randomOperator();

    // Mutation strategies
    std::string variableGibberishStrategy(std::string const& content);
    std::string invertConditionalsStrategy(std::string const& content);
    std::string swapFunctionCallsStrategy(std::string const& content);
    std::string deleteRandomLinesStrategy(std::string const& content);
    std::string swapCodeLinesStrategy(std::string const& content);
    std::string replaceOperatorsStrategy(std::string const& content);
    std::string randomCommentOutStrategy(std::string const& content);
    std::string destroyFormattingStrategy(std::string const& content);
    std::string insertUselessFunctionsStrategy(std::string const& content);
    std::string printJuliaSet(std::string const& content);
    std::string printErrorMessage(std::string const& content);

public:
    Mutator(char const* source, std::string path);
    void mutateCode();
};
#endif