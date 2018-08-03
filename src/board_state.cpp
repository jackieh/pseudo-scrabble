#include <algorithm>
#include <assert.h>
#include <iomanip>
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
    first_word_(true), num_rows_(rows), num_cols_(cols),
    board_cells_(std::vector<std::vector<BoardLetter> >()),
    moves_since_last_commit_(std::vector<BoardMove>()),
    moves_before_last_commit_(std::set<BoardMove>()),
    dictionary_(WordValidator())
{
    for (size_t rowIdx = 0; rowIdx < rows; ++rowIdx) {
        board_cells_.push_back(std::vector<BoardLetter>());
        for (size_t colIdx = 0; colIdx < cols; ++colIdx) {
            board_cells_[rowIdx].push_back(std::nullopt);
        }
    }
}

BoardState::~BoardState() { }

bool BoardState::set_cell(int row, int col, char letter,
                          std::stringstream &error_stream)
{
    if ((row < 0) || ((size_t)row >= num_rows_)) {
        error_stream << "Row \"" << std::to_string(row)
            << "\" is out of bounds";
        return false;
    }
    if ((col < 0) || ((size_t)col >= num_cols_)) {
        error_stream << "Column \"" << std::to_string(col)
            << "\" is out of bounds";
        return false;
    }
    if (!is_valid_letter(letter)) {
        error_stream << "\"" << letter << "\" is not a letter";
        return false;
    }
    if (board_cells_[row][col].has_value()) {
        error_stream << "Board cell at row " << row << " and column " << col
            << " already has a letter";
        return false;
    }
    board_cells_[row][col] = std::optional<char>(letter);
    BoardMove move = {
        .row = (size_t)row, .col = (size_t)col, .letter = letter
    };
    moves_since_last_commit_.push_back(move);
    return true;
}

// A review of the cases for identifying a valid or invalid move:
//
// Case 1: No letters placed since previous move.
//
// Cases 2 and 3: First move on the board is the placement of a single
// letter which makes up either (2) a valid word or (3) an invalid word.
//
// Case 4: A move on the board consists of multiple letters which have
// not been placed in a line.
//
// Cases 5 and 6: A move on the board consists of multiple letters on
// the same (5) row or (6) column which do not make up a contiguous line
// of letters.
//
// Cases 7 and 8: First move on the board is the placement of a series
// of multiple letters which make up either (7) a valid word or
// (8) an invalid word.
//
// Case 9: A subsequent move on the board does not have any letters
// connected to a letter from a previous move.
//
// Cases 10 and 11: A subsequent move on the board makes up at
// least one new word, either (10) at least one of which is invalid,
// or (11) all of which are valid.
//
// Return true if the letter placements make up a valid move,
// and return false otherwise.
bool BoardState::check_moves(std::stringstream &error_stream) {
    if (moves_since_last_commit_.size() == 0) {
        // Case 1: No letters placed since previous move.
        error_stream << "No letters have been placed since the last move";
        return false;
    }

    // If the first word is a single letter, then the validity of the move
    // is determined solely by the existence of that single letter as a
    // word in the dictionary.
    if (first_word_ && (moves_since_last_commit_.size() == 1)) {
        BoardMove move = moves_since_last_commit_.front();
        std::string maybe_word(&(move.letter), 1);
        if (dictionary_.is_valid(maybe_word)) {
            // Case 2: First move on the board is the placement of a
            // single letter which makes up a valid word.
            return true;
        } else {
            // Case 3: First move on the board is the placement of a
            // single letter which makes up an invalid word.
            error_stream << std::quoted(maybe_word) << " is not a word";
            return false;
        }
    }

    // Determine if letters have been placed in a straight line.
    size_t first_move_row = moves_since_last_commit_.front().row;
    size_t first_move_col = moves_since_last_commit_.front().col;
    bool same_row = true;
    bool same_col = true;
    for (auto const &move : moves_since_last_commit_) {
        same_row &= (first_move_row == move.row);
        same_col &= (first_move_col == move.col);
    }
    if (!same_row && !same_col) {
        assert(moves_since_last_commit_.size() > 1);
        // Case 4: A move on the board consists of multiple
        // letters which have not been placed in a line.
        error_stream << "Letters have not been placed in a line";
        return false;
    }

    // Determine if letter placements make up a contiguous line of letters.
    // Search for a broken up horizontal line of letters.
    if (same_row && (moves_since_last_commit_.size() > 1)) {
        // Sort moves by column.
        std::sort(moves_since_last_commit_.begin(),
                  moves_since_last_commit_.end(),
                  [](const BoardMove m1, const BoardMove m2) -> bool {
                    return m1.col < m2.col;
                  });
        std::optional<size_t> prev_col = std::nullopt;
        for (const auto &move : moves_since_last_commit_) {
            if (prev_col.has_value()
                && (move.col != (prev_col.value() + 1)))
            {
                for (size_t c_idx = prev_col.value() + 1;
                     c_idx < move.col; ++c_idx)
                {
                    if (!board_cells_[first_move_row][c_idx].has_value()) {
                        // Case 5: A move on the board consists of
                        // multiple letters on the same row which do
                        // not make up a contiguous line of letters.
                        error_stream << "Letters placed on the "
                            << "same row do not make up a contiguous "
                            << "horizontal line of letters on the board";
                            return false;
                    }
                }
                prev_col = std::optional<size_t>(move.col);
            }
        }
    }
    // Search for a broken up vertical line of letters.
    if (same_col && (moves_since_last_commit_.size() > 1)) {
        // Sort moves by row.
        std::sort(moves_since_last_commit_.begin(),
                  moves_since_last_commit_.end(),
                  [](const BoardMove m1, const BoardMove m2) -> bool {
                    return m1.row < m2.row;
                  });
        std::optional<size_t> prev_row = std::nullopt;
        for (const auto &move : moves_since_last_commit_) {
            if (prev_row.has_value()
                && (move.row != (prev_row.value() + 1)))
            {
                for (size_t r_idx = prev_row.value() + 1;
                     r_idx < move.row; ++r_idx)
                {
                    if (!board_cells_[r_idx][move.col].has_value()) {
                        // Case 6: A move on the board consists of
                        // multiple letters on the same column which
                        // do not make up a contiguous line of letters.
                        error_stream << "Letters placed on the same "
                            << "column do not make up a contiguous "
                            << "vertical line of letters on the board";
                            return false;
                    }
                }
            }
            prev_row = std::optional<size_t>(move.row);
        }
    }

    // If the first word is a straight and contiguous line of letters, then
    // the validity of the move is determined solely by the existence of
    // that series of letters as a word in the dictionary.
    if (first_word_ && (moves_since_last_commit_.size() > 1)) {
        std::string maybe_word;
        if (same_row) {
            assert(!same_col);
            assert(find_horizontal_word(
                    maybe_word, first_move_row, first_move_col));
        } else if (same_col) {
            assert(!same_row);
            assert(find_vertical_word(
                    maybe_word, first_move_row, first_move_col));
        } else {
            // The previous code should have already ruled out
            // this control flow path.
            assert(0);
        }
        if (dictionary_.is_valid(maybe_word)) {
            // Case 7: The first word on the board is the placement
            // of letters which make up a valid word.
            return true;
        } else {
            // Case 8: The first word on the board is the placement
            // of letters which make up an invalid word.
            error_stream << std::quoted(maybe_word) << " is not a word";
            return false;
        }
    }

    // First word cases should be ruled out.
    // Now assume previous moves exist on the board.
    assert(!first_word_);
    assert(moves_before_last_commit_.size() > 0);

    // Determine connection to previously existing letters.
    bool horiz_adjacent_to_prev = false;
    bool vert_adjacent_to_prev = false;
    for (const auto &move : moves_since_last_commit_) {
        horiz_adjacent_to_prev
            |= has_prev_horiz_neighbor(move.row, move.col);
        vert_adjacent_to_prev
            |= has_prev_vert_neighbor(move.row, move.col);
    }
    if (!horiz_adjacent_to_prev && !vert_adjacent_to_prev) {
        // Case 9: A subsequent move on the board does not have any
        // letters connected to a letter from a previous move.
        error_stream << "No letters since previous successful "
            << "move connected to existing word";
        return false;
    }

    // Search for potentially multiple words for each set of
    // adjacent letters in the series of letter placements.
    // Find out if any of these words are not valid words.
    std::set<std::string> maybe_words;
    std::vector<std::string> not_words;
    for (auto const &move : moves_since_last_commit_) {
        std::string maybe_word;
        if (has_prev_horiz_neighbor(move.row, move.col)) {
            assert(find_horizontal_word(maybe_word, move.row, move.col));
            maybe_words.insert(maybe_word);
        }
        if (has_prev_vert_neighbor(move.row, move.col)) {
            assert(find_vertical_word(maybe_word, move.row, move.col));
            maybe_words.insert(maybe_word);
        }
    }
    // Account for the possibility that the line of letters may not
    // necessarily be adjacent to a letter from a previous move in
    // the particular direction of the line, but only if the letters
    // are adjacent to each other (in other words, multiple letters
    // have been placed).
    {
        std::string maybe_word;
        if (moves_since_last_commit_.size() > 1) {
            if (same_row) {
                assert(find_horizontal_word(
                        maybe_word, first_move_row, first_move_col));
                maybe_words.insert(maybe_word);
            } else if (same_col) {
                assert(find_vertical_word(
                        maybe_word, first_move_row, first_move_col));
                maybe_words.insert(maybe_word);
            }
        }
    }
    assert(maybe_words.size() > 0);
    for (const auto &maybe_word : maybe_words) {
        if (!dictionary_.is_valid(maybe_word)) {
            not_words.push_back(maybe_word);
        }
    }
    if (not_words.size() > 0) {
        // Case 10: A subsequent move on the board makes up at least
        // one new word, at least one of which is invalid.
        bool plural = (not_words.size() > 1);
        error_stream << (plural ? "Words" : "Word")
            << " from adjacent letters ";
        bool first_not_word = true;
        for (const auto &not_word : not_words) {
            if (first_not_word) {
                first_not_word = false;
            } else {
                error_stream << ", ";
            }
            error_stream << std::quoted(not_word);
        }
        error_stream << (plural ? " are not valid words"
                         : " is not a valid word");
        return false;
    } else {
        // Case 11: A subsequent move on the board makes up at least
        // one new word, all of which are valid.
        return true;
    }
}

bool BoardState::has_prev_horiz_neighbor(size_t row, size_t col) {
    bool west_adjacent = false;
    if (col > 0) {
        BoardLetter west_letter = board_cells_[row][col - 1];
        if (west_letter.has_value()) {
            BoardMove west_move = {row, col - 1, west_letter.value()};
            if (moves_before_last_commit_.find(west_move)
                != moves_before_last_commit_.end())
            {
                west_adjacent = true;
            }
        }
    }
    bool east_adjacent = false;
    if (col < num_cols_) {
        BoardLetter east_letter = board_cells_[row][col + 1];
        if (east_letter.has_value()) {
            BoardMove east_move = {row, col + 1, east_letter.value()};
            if (moves_before_last_commit_.find(east_move)
                != moves_before_last_commit_.end())
            {
                east_adjacent = true;
            }
        }
    }
    return (east_adjacent || west_adjacent);
}

bool BoardState::has_prev_vert_neighbor(size_t row, size_t col)
{
    bool north_adjacent = false;
    if (row > 0) {
        BoardLetter north_letter = board_cells_[row - 1][col];
        if (north_letter.has_value()) {
            BoardMove north_move = {row - 1, col, north_letter.value()};
            if (moves_before_last_commit_.find(north_move)
                != moves_before_last_commit_.end())
            {
                north_adjacent = true;
            }
        }
    }
    bool south_adjacent = false;
    if (row < num_rows_) {
        BoardLetter south_letter = board_cells_[row + 1][col];
        if (south_letter.has_value()) {
            BoardMove south_move = {row + 1, col, south_letter.value()};
            if (moves_before_last_commit_.find(south_move)
                != moves_before_last_commit_.end())
            {
                south_adjacent = true;
            }
        }
    }
    return (north_adjacent || south_adjacent);
}

bool BoardState::find_horizontal_word(std::string &maybe_word,
                                      size_t row, size_t col)
{
    if (!board_cells_[row][col].has_value()) {
        return false;
    }
    std::vector<char> horizontal_letters;
    // Modify column index until we find the leftmost letter.
    size_t colIdx;
    for (colIdx = col;
         (colIdx > 0) && board_cells_[row][colIdx - 1].has_value();
         --colIdx);
    // Append letters to the maybe word.
    for (; ((colIdx < num_cols_) && board_cells_[row][colIdx].has_value());
         ++colIdx)
    {
        horizontal_letters.push_back(board_cells_[row][colIdx].value());
    }
    horizontal_letters.push_back(0);
    maybe_word = std::string(horizontal_letters.begin(),
                             horizontal_letters.end());
    return true;
}

bool BoardState::find_vertical_word(std::string &maybe_word,
                                    size_t row, size_t col)
{
    if (!board_cells_[row][col].has_value()) {
        return false;
    }
    std::vector<char> vertical_letters;
    // Modify row index until we find the topmost letter.
    size_t rowIdx;
    for (rowIdx = row;
         (rowIdx > 0) && board_cells_[rowIdx - 1][col].has_value();
         --rowIdx);
    // Append letters to the maybe word.
    for (; ((rowIdx < num_rows_) && board_cells_[rowIdx][col].has_value());
         ++rowIdx)
    {
        vertical_letters.push_back(board_cells_[rowIdx][col].value());
    }
    vertical_letters.push_back(0);
    maybe_word = std::string(vertical_letters.begin(),
                             vertical_letters.end());
    return true;
}

void BoardState::clear() {
    for (auto &board_cells_row : board_cells_) {
        for (auto &board_cell : board_cells_row) {
            board_cell = std::nullopt;
        }
    }
    moves_since_last_commit_.clear();
    moves_before_last_commit_.clear();
    first_word_ = true;
}

void BoardState::commit() {
    moves_before_last_commit_.insert(moves_since_last_commit_.begin(),
                                     moves_since_last_commit_.end());
    moves_since_last_commit_.clear();
    first_word_ = false;
}

void BoardState::revert() {
    for (auto const &move : moves_since_last_commit_) {
        board_cells_[move.row][move.col] = std::nullopt;
    }
    moves_since_last_commit_.clear();
}

BoardState::BoardLetter BoardState::get_maybe_letter(int row, int col) const {
    if ((row < 0) || ((size_t)row >= num_rows_)
        || (col < 0) || ((size_t)col >= num_cols_))
    {
        return std::nullopt;
    } else {
        return board_cells_[row][col];
    }
}
