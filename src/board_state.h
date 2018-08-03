#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include <optional>
#include <set>
#include <sstream>
#include <vector>

#include <word_validator.h>

class BoardState {
public:
    typedef std::optional<char> BoardLetter;
    static bool is_valid_letter(char letter);

    typedef struct BoardMove {
        size_t row;
        size_t col;
        char letter;
        bool operator<(struct BoardMove other) const {
            return (row == other.row) ? (col < other.col) : (row < other.row);
        }
    } BoardMove;

    BoardState(size_t rows, size_t cols);
    ~BoardState();

    bool set_cell(int row, int col, char letter,
                  std::stringstream &error_stream);
    bool check_moves(std::stringstream &error_stream);
    void clear();
    void commit();
    void revert();

    BoardLetter get_maybe_letter(int row, int col) const;

private:
    bool has_prev_vert_neighbor(size_t row, size_t col);
    bool has_prev_horiz_neighbor(size_t row, size_t col);

    bool find_horizontal_word(std::string &maybe_word, size_t row, size_t col);
    bool find_vertical_word(std::string &maybe_word, size_t row, size_t col);

    bool first_word_;
    size_t num_rows_;
    size_t num_cols_;
    std::vector<std::vector<BoardLetter> > board_cells_;
    std::vector<BoardMove> moves_since_last_commit_;
    std::set<BoardMove> moves_before_last_commit_;
    WordValidator dictionary_;
};

#endif // BOARDSTATE_H
