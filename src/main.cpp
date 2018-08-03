#include <assert.h>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <optional>
#include <signal.h>
#include <sstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <board_state.h>

namespace bpo = boost::program_options;

class PseudoScrabble {
public:
    static constexpr int default_rows = 19;
    static constexpr int default_cols = 19;

    PseudoScrabble() :
        help_opt_(false),
        rows_opt_(std::nullopt),
        cols_opt_(std::nullopt),
        options_string_(std::string())
    { }

    int parse_options(int argc, char **argv) {
        try {
            bpo::options_description opt_descr("Arguments");
            create_options(opt_descr);
            bpo::variables_map var_map;
            bpo::command_line_parser parser(argc, argv);
            auto &parser_opt = parser.options(opt_descr);
            bpo::store(parser_opt.run(), var_map);
            bpo::notify(var_map);
            set_options(var_map);
        } catch (bpo::multiple_occurrences &error) {
            // Option X cannot be specified more than once.
            std::cerr << "Error: " << error.what() << std::endl;
            return exit_more_information();
        } catch (bpo::unknown_option &error) {
            // Unrecognized option X.
            std::cerr << "Error: Unrecognized option \""
                << error.get_option_name() << "\"" << std::endl;
            return exit_more_information();
        } catch (bpo::invalid_command_line_syntax &error) {
            std::cerr << "Error: Failed to interpret an argument after the "
                << error.get_option_name() << " option" << std::endl;
            return exit_more_information();
        }

        if (help_opt_) {
            std::cerr << usage_string() << std::endl
                << examples_string() << std::endl
                << options_string_ << std::endl;
            return 1;
        }

        return 0;
    }

    // Evaluate the collected command line options and export an image.
    // Return 0 if successful, or return a nonzero int if a fatal error is
    // encountered.
    //
    // Check for help, check for output file, then create an image and set
    // stripe colors/width/length/orientation and export the image to the
    // output file.
    int exec_game() {
        int board_rows = rows_opt_.value_or(default_rows);
        int board_cols = cols_opt_.value_or(default_cols);

        // Validate command line input.
        bool bad_dimensions = false;
        if (board_rows <= 0) {
            std::cerr << "Error: Can't play a game with "
                << board_rows << " rows, please specify a number of "
                << "rows that is a positive integer" << std::endl;
            bad_dimensions = true;
        }
        if (board_cols <= 0) {
            std::cerr << "Error: Can't play a game with "
                << board_cols << " columns, please specify a number of "
                << "columns that is a positive integer" << std::endl;
            bad_dimensions = true;
        }
        if (bad_dimensions) {
            return exit_more_information();
        }

        // Initialize game.
        BoardState board((size_t)board_rows, (size_t)board_cols);
        print_game_welcome();
        size_t move_count = 0;

        // Don't exit on ctrl-C. The reason for this is to make the
        // experience similar to any other REPL like the Bash command
        // line interface or a language interpreter, where ctrl-C can be
        // used to discard the characters typed so far on the current
        // prompt and begin a new prompt line.
        signal(SIGINT, PseudoScrabble::sig_int_handler);

        // Run game loop and return when told to quit.
        for (;;) {
            print_repl_prompt();
            std::string input;
            if (!std::getline(std::cin, input)) {
                // Quit when stdin gives me EOF, which we expect to
                // be triggered by ctrl-D.
                return exit_repl();
            }

            // Parse game command input into tokens delimited by whitespace.
            std::vector<std::string> tokens;
            std::istringstream token_stream(input);
            for (std::string token_from_stream;
                 token_stream >> token_from_stream;
                 tokens.push_back(token_from_stream));

            if (tokens.size() == 0) {
                // User pressed enter without any non-whitespace content.
                continue;
            } else {
                std::string operation = tokens.front();
                // Process game command input.
                if (operation.compare("help") == 0) {
                    // Print help for commands.
                    ignore_operands_if_any(tokens);
                    print_game_help();
                } else if (operation.compare("quit") == 0) {
                    // Quit the game.
                    ignore_operands_if_any(tokens);
                    return exit_repl();
                } else if (operation.compare("clear") == 0) {
                    // Clear the board.
                    ignore_operands_if_any(tokens);
                    board.clear();
                    std::cout << "Board has been cleared"
                        << std::endl << std::endl;
                } else if (operation.compare("place") == 0) {
                    // Place a letter on the board.
                    // Parse letter operand.
                    std::optional<char> letter_operand =
                        parse_letter_operand(tokens);
                    if (!letter_operand.has_value()) {
                        continue;
                    }
                    // Parse row operand.
                    std::optional<int> row_operand =
                        parse_row_operand(tokens, board_rows);
                    if (!row_operand.has_value()) {
                        continue;
                    }
                    // Parse column operand.
                    std::optional<int> col_operand =
                        parse_col_operand(tokens, board_cols);
                    if (!col_operand.has_value()) {
                        continue;
                    }
                    // Try to place the letter and report error if applicable.
                    std::stringstream bad_placement_stream;
                    if (board.set_cell(row_operand.value(),
                                       col_operand.value(),
                                       letter_operand.value(),
                                       bad_placement_stream))
                    {
                        std::cout << "Letter has been placed on the board"
                            << std::endl << std::endl;
                    } else {
                        std::cout << "Bad placement: "
                            << bad_placement_stream.str() << std::endl;
                    }
                } else if (operation.compare("submit") == 0) {
                    // Try to submit a move.
                    ignore_operands_if_any(tokens);
                    std::stringstream bad_move_stream;
                    if (board.check_moves(bad_move_stream)) {
                        // If true then the move is good.
                        board.commit();
                        ++move_count;
                        std::cout << "Move successful; " << move_count
                            << " " << ((move_count == 1) ? "move" : "moves")
                            << " made so far" << std::endl << std::endl;
                    } else {
                        // If false then explain why the move is not good.
                        std::cout << "Move failed; "
                            << bad_move_stream.str() << std::endl << std::endl;
                    }
                } else if (operation.compare("revert") == 0) {
                    // Revert the board to the previous move.
                    ignore_operands_if_any(tokens);
                    board.revert();
                    std::cout << "Board has been reverted to the previous move"
                        << std::endl << std::endl;
                } else if (operation.compare("print") == 0) {
                    // Print the board as a grid to stdout.
                    ignore_operands_if_any(tokens);
                    std::cout << std::endl<< "Moves made: "
                        << move_count << std::endl << std::endl;
                    print_grid_top_or_bottom((size_t)board_cols);
                    bool first_row = true;
                    for (int r = 0; r < board_rows; ++r) {
                        bool first_col;
                        if (first_row) {
                            first_row = false;
                        } else {
                            // Print separating horizontal line between rows.
                            print_grid_horizontal_line((size_t)board_cols);
                        }
                        // Print row of board cells.
                        std::cout << "|";
                        first_col = true;
                        for (int c = 0; c < board_cols; ++c) {
                            if (first_col) {
                                first_col = false;
                            } else {
                                std::cout << "|";
                            }
                            const BoardState::BoardLetter maybe_letter =
                                board.get_maybe_letter((size_t)r, (size_t)c);
                            std::cout << maybe_letter.value_or(' ');
                        }
                        std::cout << "|" << std::endl;
                    }
                    print_grid_top_or_bottom((size_t)board_cols);
                    std::cout << std::endl;
                } else {
                    std::cout << operation << ": command not found"
                        << std::endl << std::endl;
                }
            }
        }
    }

private:
    std::optional<char> parse_letter_operand(
        std::vector<std::string> &tokens)
    {
        std::optional<int> row_operand = std::nullopt;
        if (tokens.size() < 2) {
            std::cout << "Invalid use of \"place\"; No letter, row, and "
                << "column specified with \"place\"" << std::endl << std::endl;
            return std::nullopt;
        }
        // Number of tokens needed to parse this operand is acceptable.
        // Find out if the operand is a letter.
        std::string letter_token = tokens[1];
        if (letter_token.length() == 1) {
            char maybe_letter = toupper(letter_token[0]);
            if (BoardState::is_valid_letter(maybe_letter)) {
                return std::optional<char>(maybe_letter);
            }
        }
        // Falling through to here means the operand is not a letter.
        std::cout << "Invalid use of \"place\"; " << std::quoted(letter_token)
            << " is not a letter" << std::endl << std::endl;
        return std::nullopt;
    }

    std::optional<int> parse_row_operand(
        std::vector<std::string> &tokens, int board_rows)
    {
        if (tokens.size() < 3) {
            std::cout << "Invalid use of \"place\"; No row and column "
                << "specified with \"place\"" << std::endl << std::endl;
            return std::nullopt;
        }
        // Number of tokens needed to parse this operand is acceptable.
        // Find out if the operand is an integer.
        std::string row_token = tokens[2];
        int maybe_row = 0;
        try {
            maybe_row = std::stoi(row_token);
        } catch (std::invalid_argument &error) {
            std::cout << "Invalid use of \"place\"; " << std::quoted(row_token)
                << " is not an integer" << std::endl << std::endl;
            return std::nullopt;
        } catch (std::out_of_range &error) {
            std::cout << "Invalid use of \"place\"; " << std::quoted(row_token)
                << " is too big to store in an integer variable"
                << std::endl << std::endl;
            return std::nullopt;
        }
        // Operand is an integer, but find out if it's an acceptable integer.
        if (maybe_row < 1) {
            std::cout << "Invalid use of \"place\"; specified row must be "
                << "a positive integer and " << std::quoted(row_token)
                << " is not a positive integer" << std::endl << std::endl;
            return std::nullopt;
        } else if (maybe_row >= board_rows) {
            std::cout << "Invalid use of \"place\"; the board doesn't have "
                << row_token << "rows" << std::endl << std::endl;
            return std::nullopt;
        }
        return std::optional<int>(maybe_row);
    }

    std::optional<int> parse_col_operand(
        std::vector<std::string> &tokens, int board_cols)
    {
        if (tokens.size() < 4) {
            std::cout << "Invalid use of \"place\"; No column "
                << "specified with \"place\"" << std::endl << std::endl;
            return std::nullopt;
        }
        // Number of tokens needed to parse this operand is acceptable.
        // Find out if the operand is an integer.
        std::string col_token = tokens[3];
        int maybe_col = 0;
        try {
            maybe_col = std::stoi(col_token);
        } catch (std::invalid_argument &error) {
            std::cout << "Invalid use of \"place\"; "
                << std::quoted(col_token)
                << " is not an integer" << std::endl << std::endl;
            return std::nullopt;
        } catch (std::out_of_range &error) {
            std::cout << "Invalid use of \"place\"; "
                << std::quoted(col_token)
                << " is too big to store in an integer variable"
                << std::endl << std::endl;
            return std::nullopt;
        }
        // Operand is an integer, but find out if it's an acceptable integer.
        if (maybe_col < 1) {
            std::cout << "Invalid use of \"place\"; specified column must be "
                << "a positive integer and " << std::quoted(col_token)
                << " is not a positive integer" << std::endl << std::endl;
            return std::nullopt;
        } else if (maybe_col >= board_cols) {
            std::cout << "Invalid use of \"place\"; the board doesn't have "
                << col_token << "columns" << std::endl << std::endl;
            return std::nullopt;
        }
        return std::optional<int>(maybe_col);
    }

    void print_grid_horizontal_line(size_t width) {
        std::cout << "+";
        bool first_col = true;
        for (size_t c = 0; c < width; ++c) {
            if (first_col) {
                first_col = false;
            } else {
                std::cout << "+";
            }
            std::cout << "-";
        }
        std::cout << "+" << std::endl;
    }

    void print_grid_top_or_bottom(size_t width) {
        std::cout << "+";
        for (size_t c = 0; c < (width + width - 1); ++c) {
            std::cout << "-";
        }
        std::cout << "+" << std::endl;
    }

    static int exit_repl() {
        std::cout << std::endl << "Goodbye" << std::endl << std::endl;
        return 0;
    }

    static void sig_int_handler(int s) {
        std::cout << std::endl
            << "Keyboard interrupt (signal " << s
            << ") caught; type \"quit\" to exit this prompt"
            << std::endl;
        print_repl_prompt();
    }

    static void print_repl_prompt() {
        std::cout << ">>> ";
        std::cout.flush();
    }

    static void print_game_welcome() {
        std::cout << "Welcome to Pseudo-Scrabble." << std::endl;
        std::cout << "Type \"help\" for instructions." << std::endl;
    }

    static void print_game_help() {
        std::cout                                                                                << std::endl
            << "Play Pseudo-Scrabble by repeatedly making moves. To make a move, place any"      << std::endl
            << "number of letters on the blank spaces of this board, then submit the move. If"   << std::endl
            << "the move is valid, then the move will be saved to the board and a score counter" << std::endl
            << "will increment. If the move is not valid, then the move is not saved and the"    << std::endl
            << "player has the option to revert the board to the previous successful move."      << std::endl
                                                                                                 << std::endl
            << "To place a letter on the board, run the \"place\" command specifying a single"   << std::endl
            << "letter, and a valid row number and column number indicating the location of"     << std::endl
            << "placement. Rows and columns are one-indexed (e.g. the first row is row 1,"       << std::endl
            << "and row 0 does not exist)."                                                      << std::endl
                                                                                                 << std::endl
            << "A valid move meets the following criteria:"                                      << std::endl
            << "- Letters must be played in a straight line, up-down or left-right."             << std::endl
            << "- The first word can be played anywhere on the board."                           << std::endl
            << "- All subsequent words must share at least one space with an existing word."     << std::endl
            << "- Word direction can be left-to-right or top-to-bottom."                         << std::endl
            << "- All sets of adjacent letters must form valid words."                           << std::endl
                                                                                                 << std::endl
            << "Description of commands"                                                         << std::endl
            << "\"help\":  Print these instructions for use."                                    << std::endl
            << "\"quit\":  Exit Pseudo-Scrabble."                                                << std::endl
            << "\"clear\": Clear the board."                                                     << std::endl
            << "\"place [L] [R] [C]\": Place a [L]etter at the specified [R]ow and [C]olumn."    << std::endl
            << "\"submit\": Evaluate letters placed on the board."                               << std::endl
            << "\"revert\": Revert the board state to the most recent successful move."          << std::endl
            << "\"print\":  Print the current board state and the number of moves made so far."  << std::endl
                                                                                                 << std::endl;
    }

    static void ignore_operands_if_any(std::vector<std::string> &tokens) {
        std::stringstream operands_stream;
        bool first_operand = true;
        for (auto operand_iter = std::next(tokens.begin());
             operand_iter != tokens.end(); ++operand_iter)
        {
            if (first_operand) {
                first_operand = false;
            } else {
                operands_stream << " ";
            }
            operands_stream << *operand_iter;
        }
        std::string operands = operands_stream.str();
        if (tokens.size() > 1) {
            std::cout << "Ignoring " << std::quoted(operands)
                << "..." << std::endl;
        }
    }

    int exit_more_information() {
        std::cerr << "Run \"" << EXEC_NAME << " " << "--help"
            << "\" for more information." << std::endl;
        return 1;
    }

    // Return summary of the functionality of this tool.
    std::string usage_string() {
        std::stringstream usage_stream;
        usage_stream << "Usage: " << EXEC_NAME
            << " [board dimensions]" << std::endl
            << "Interact with a pseudo-Scrabble board via a REPL."
            << std::endl;
        return usage_stream.str();
    }

    // Return description of specific examples of launching the REPL.
    std::string examples_string() {
        std::stringstream examples_stream;
        examples_stream << "Examples: " << EXEC_NAME << std::endl;
        examples_stream << "      or: " << EXEC_NAME
            << " -r 10 -c 20" << std::endl;
        return examples_stream.str();
    }

    // Create names and a description for each command line option.
    void create_options(bpo::options_description &opt) {
        const char *help_chars = "Print this help message and exit";

        std::stringstream rows_stream;
        rows_stream << "Specify number of rows in the board"
            << " (default " << default_rows << ")";
        std::string rows_string = rows_stream.str();
        const char *rows_chars = rows_string.c_str();
        const auto *rows_semantic(bpo::value<int>());

        std::stringstream cols_stream;
        cols_stream << "Specify number of columns in the board"
            << " (default " << default_cols << ")";
        std::string cols_string = cols_stream.str();
        const char *cols_chars = cols_string.c_str();
        const auto *cols_semantic(bpo::value<int>());

        opt.add_options()
            ("help,h", help_chars)
            ("rows,r", rows_semantic, rows_chars)
            ("cols,c", cols_semantic, cols_chars)
        ;

        std::stringstream options_stream;
        options_stream << opt;
        options_string_ = options_stream.str();
    }

    // Set private fields of this MkStripes object from the command line
    // options.
    void set_options(bpo::variables_map var_map) {
        help_opt_ |= !var_map["help"].empty();
        if (!var_map["rows"].empty()) {
            rows_opt_ = std::optional<int>(var_map["rows"].as<int>());
        }
        if (!var_map["cols"].empty()) {
            cols_opt_ = std::optional<int>(var_map["cols"].as<int>());
        }
    }

    bool help_opt_;
    std::optional<int> rows_opt_;
    std::optional<int> cols_opt_;
    std::string options_string_;
};

int main(int argc, char **argv) {
    PseudoScrabble pseudo_scrabble_state;
    int parse_options_result = pseudo_scrabble_state.parse_options(argc, argv);
    return ((parse_options_result == 0)
            ? pseudo_scrabble_state.exec_game() : parse_options_result);
}
