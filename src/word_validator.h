#ifndef WORDVALIDATOR_H
#define WORDVALIDATOR_H

#include <string>

#include <aspell.h>

class WordValidator {
public:
    WordValidator();
    ~WordValidator();
    bool is_valid(const std::string &word) const;
private:
    AspellConfig *spell_config_;
    AspellSpeller *spell_checker_;
};

#endif // WORDVALIDATOR_H
