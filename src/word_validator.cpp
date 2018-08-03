#include <iostream>
#include <string>

#include <aspell.h>
#include <word_validator.h>

WordValidator::WordValidator() :
    spell_config_(new_aspell_config()),
    spell_checker_(0)
{
    aspell_config_replace(spell_config_, "lang", "en_US");
    AspellCanHaveError *possible_error = new_aspell_speller(spell_config_);
    if (aspell_error_number(possible_error) != 0) {
        std::cout << aspell_error_message(possible_error);
    } else {
        spell_checker_ = to_aspell_speller(possible_error);
    }
}

WordValidator::~WordValidator() {
    delete_aspell_speller(spell_checker_);
    delete_aspell_config(spell_config_);
}

bool WordValidator::is_valid(const std::string &word) const {
    int correct = aspell_speller_check(
                spell_checker_, word.c_str(), word.length());
    return (correct != 0);
}
