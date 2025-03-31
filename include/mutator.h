#ifndef MUTATOR_H
#define MUTATOR_H

class Mutator {
private:
    char const* source;

public:
    Mutator(char const* source);
    void printSource();
};

#endif