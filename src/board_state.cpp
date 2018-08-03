#include <assert.h>
#include <optional>
#include <sstream>
#include <vector>
#include <board_state.h>
#include <word_validator.h>

bool BoardState::is_valid_letter(char letter) {
    return ((letter == 'A') || (letter == 'B') || (letter == 'C')
            || (letter == 'D') || (letter == 'E') || (letter == 'F')
            || (letter == 'G') || (letter == 'H') || (letter == 'I')
            || (letter == 'J') || (letter == 'K') || (letter == 'L')
            || (letter == 'M') || (letter == 'N') || (letter == 'O')
            || (letter == 'P') || (letter == 'Q') || (letter == 'R')
            || (letter == 'S') || (letter == 'T') || (letter == 'U')
            || (letter == 'V') || (letter == 'W') || (letter == 'X')
            || (letter == 'Y') || (letter == 'Z'));
}

BoardState::BoardState(size_t rows, size_t cols):
    first_word(true), num_rows(rows), num_cols(cols),
    board_cells(std::vector<std::vector<board_letter> >()),
    moves_since_last_commit(std::vector<BoardMove>()),
    dictionary(WordValidator())
{
    for (size_t rowIdx = 0; rowIdx < rows; ++rowIdx) {
        board_cells.push_back(std::vector<board_letter>());
        for (size_t colIdx = 0; colIdx < cols; ++colIdx) {
            board_cells[rowIdx].push_back(std::nullopt);
        }
    }
}

BoardState::~BoardState() { }

bool BoardState::set_cell(int row, int col, char letter,
                          std::stringstream &error_stream)
{
    if ((row < 0) || ((size_t)row >= num_rows)) {
        error_stream << "Row \"" << std::to_string(row)
            << "\" is out of bounds";
        return false;
    }
    if ((col < 0) || ((size_t)col >= num_cols)) {
        error_stream << "Column \"" << std::to_string(col)
            << "\" is out of bounds";
        return false;
    }
    if (!is_valid_letter(letter)) {
        error_stream << "\"" << letter << "\" is not a letter";
        return false;
    }
    if (board_cells[row][col].has_value()) {
        error_stream << "Board cell at row " << row << " and column " << col
            << " already has a letter";
        return false;
    }
    board_cells[row][col] = std::optional<char>(letter);
    BoardMove move = {
        .row = (size_t)row, .col = (size_t)col, .letter = letter
    };
    moves_since_last_commit.push_back(move);
    return true;
}

bool BoardState::check_moves(std::stringstream &error_stream) {
    if (moves_since_last_commit.size() == 0) {
        error_stream << "No letters have been placed since the last move";
        return false;
    }

    if (moves_since_last_commit.size() == 1) {
        BoardMove move = moves_since_last_commit.front();
        bool horizontal_adjacent = adjacent_horizontal(move.row, move.col);
        bool vertical_adjacent = adjacent_vertical(move.row, move.col);
        if (!horizontal_adjacent && !vertical_adjacent && !first_word) {
            // The move is invalid when it is not connected to an existing
            // word, unless this is the first move on the board since it
            // was initialized or cleared.
            error_stream << "Letter not connected to existing word";
            return false;
        }
        if (!check_horizontal_word(move.row, move.col)) {
            // If a new set of adjacent letters is made that does
            // not form a valid word, then this is not a valid move.
            error_stream << "Horizontal adjacent letters on row "
                << std::to_string(move.row) << " are not a word";
            return false;
        }
        if (!check_vertical_word(move.row, move.col)) {
            // If a new set of adjacent letters is made that does
            // not form a valid word, then this is not a valid move.
            error_stream << "Vertical adjacent letters on column "
                << std::to_string(move.col) << " are not a word";
            return false;
        }
    }

    // More than one letter has been placed.
    bool same_row = true;
    bool same_col = true;
    std::optional<size_t> prev_row = std::nullopt;
    std::optional<size_t> prev_col = std::nullopt;
    for (auto const &move : moves_since_last_commit) {
        if (prev_row.has_value()) {
            same_row &= (move.row == prev_row);
        }
        prev_row = move.row;
        if (prev_col.has_value()) {
            same_col &= (move.col == prev_col);
        }
        prev_col = move.col;
    }
    if (!same_row && !same_col) {
        error_stream << "Letters have not been placed in a line";
        return false;
    }
    size_t row = prev_row.value();
    size_t col = prev_col.value();
    if (same_row) {
        // The first move may be a single letter word, but otherwise the
        // line cannot be both horizontal and vertical.
        assert((first_word && moves_since_last_commit.size() == 1)
               || !same_col);
        if (!check_horizontal_word(row, col)) {
            error_stream << "Horizontal adjacent letters on row "
                << std::to_string(row) << " are not a word";
            return false;
        }
        for (auto const &move : moves_since_last_commit) {
            if (!check_vertical_word(move.row, move.col)) {
                error_stream << "Vertical adjacent letters on column "
                    << std::to_string(move.col) << " are not a word";
                return false;
            }
        }
        // All the moves check out.
        return true;
    } else {
        assert(same_col);
        if (!check_vertical_word(row, col)) {
            error_stream << "Vertical adjacent letters on column "
                << std::to_string(col) << " are not a word";
            return false;
        }
        for (auto const &move : moves_since_last_commit) {
            if (!check_horizontal_word(move.row, move.col)) {
                error_stream << "Horizontal adjacent letters on row "
                    << std::to_string(move.row) << " are not a word";
                return false;
            }
        }
        // All the moves check out.
        return true;
    }
}

bool BoardState::adjacent_horizontal(size_t row, size_t col)
{
    bool west_adjacent = ((col > 0) && board_cells[row][col - 1].has_value());
    bool east_adjacent = ((col < num_cols)
                          && board_cells[row][col + 1].has_value());
    return (east_adjacent || west_adjacent);
}

bool BoardState::adjacent_vertical(size_t row, size_t col)
{
    bool north_adjacent = ((row > 0) && board_cells[row - 1][col].has_value());
    bool south_adjacent = ((row < num_rows)
                           && board_cells[row + 1][col].has_value());
    return (north_adjacent || south_adjacent);
}

bool BoardState::check_horizontal_word(size_t row, size_t col)
{
    if (adjacent_horizontal(row, col)) {
        std::vector<char> horizontal_letters;
        int rowIdx = (int)row;
        // Modify row index until we find the leftmost letter.
        for (; ((rowIdx >= 0) && board_cells[rowIdx][col].has_value());
             --rowIdx);
        // Append letters to the test word.
        for (; ((rowIdx < (int)num_cols)
                && board_cells[rowIdx][row].has_value());
             ++rowIdx)
        {
            horizontal_letters.push_back(board_cells[rowIdx][row].value());
        }
        std::string horizontal_word(horizontal_letters.begin(),
                                    horizontal_letters.end());
        return dictionary.is_valid(horizontal_word);
    }
    // We don't have a problem if there are no horizontal-adjacent letters.
    return true;
}

bool BoardState::check_vertical_word(size_t row, size_t col)
{
    if (adjacent_vertical(row, col)) {
        std::vector<char> vertical_letters;
        int colIdx = (int)col;
        // Modify column index until we find the topmost letter.
        for (; ((colIdx >= 0)
                && board_cells[row][colIdx].has_value());
             --colIdx);
        // Append letters to the test word.
        for (; ((colIdx < (int)num_cols)
                && board_cells[row][colIdx].has_value());
             ++colIdx)
        {
            vertical_letters.push_back(board_cells[row][colIdx].value());
        }
        std::string vertical_word(vertical_letters.begin(),
                                  vertical_letters.end());
        return dictionary.is_valid(vertical_word);
    }
    // We don't have a problem if there are no vertical-adjacent letters.
    return true;
}

void BoardState::clear() {
    for (auto &board_cells_row : board_cells) {
        for (auto &board_cell : board_cells_row) {
            board_cell.reset();
        }
    }
    first_word = true;
}

void BoardState::commit() {
    moves_since_last_commit.clear();
    first_word = false;
}

void BoardState::revert() {
    for (auto const &move : moves_since_last_commit) {
        board_cells[move.row][move.col].reset();
    }
}

BoardState::board_letter BoardState::get_maybe_letter(int row, int col) const {
    if ((row < 0) || ((size_t)row >= num_rows)
        || (col < 0) || ((size_t)col >= num_cols))
    {
        return std::nullopt;
    } else {
        return board_cells[row][col];
    }
}
