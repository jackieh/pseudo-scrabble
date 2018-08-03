#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include <optional>
#include <sstream>
#include <vector>
#include <word_validator.h>


class BoardState {
public:
    typedef std::optional<char> board_letter;
    static bool is_valid_letter(char letter);

    typedef struct BoardMove {
        size_t row;
        size_t col;
        char letter;
    } BoardMove;

    BoardState(size_t rows, size_t cols);
    ~BoardState();

    bool set_cell(int row, int col, char letter,
                  std::stringstream &error_stream);
    bool check_moves(std::stringstream &error_stream);
    void clear();
    void commit();
    void revert();

    board_letter get_maybe_letter(int row, int col) const;

private:
    bool adjacent_horizontal(size_t row, size_t col);
    bool adjacent_vertical(size_t row, size_t col);
    bool check_horizontal_word(size_t row, size_t col);
    bool check_vertical_word(size_t row, size_t col);
    bool first_word;
    size_t num_rows;
    size_t num_cols;
    std::vector<std::vector<board_letter> > board_cells;
    std::vector<BoardMove> moves_since_last_commit;
    WordValidator dictionary;
};

#endif // BOARDSTATE_H
