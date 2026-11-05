#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <limits>
#include <cstdlib>
#include <cctype>
using namespace std;

const int MAX_QUESTIONS = 3;   // most questions allowed per game
const int WIDTH = 50;         

//  Colors 
const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string RED    = "\033[31m";
const string GREEN  = "\033[32m";
const string YELLOW = "\033[33m";
const string CYAN   = "\033[36m";
const string GRAY   = "\033[90m";

//  Class: one quiz question 
class Question {
private:
    string category;
    string difficulty;
    string text;
    string choices[4];   // array of 4 answers
    int correct;         // 1-4

public:
    // Constructor: sets up a question when it is created
    Question(string cat, string diff, string t,
             string a, string b, string c, string d, int right) {
        category = cat;
        difficulty = diff;
        text = t;
        choices[0] = a;
        choices[1] = b;
        choices[2] = c;
        choices[3] = d;
        correct = right;
    }

    // Getters: let other code read the private data
    string getCategory() const { return category; }
    string getDifficulty() const { return difficulty; }
    string getText() const { return text; }
    string getChoice(int i) const { return choices[i]; }
    int getCorrect() const { return correct; }
};



void clearScreen() {
    cout << "\033[2J\033[H"; 
}


string repeatChar(char c, int times) {
    string result = "";
    for (int i = 0; i < times; i++)
        result += c;
    return result;
}

// Prints  +------------------------------------------------+
void printLine() {
    cout << CYAN << "+" << repeatChar('-', WIDTH - 2) << "+" << RESET << "\n";
}

// Prints a title centered inside a box
void printTitle(const string& title) {
    int space = WIDTH - 2 - title.length();
    int left = space / 2;
    int right = space - left;

    printLine();
    cout << CYAN << "|" << RESET
         << repeatChar(' ', left) << BOLD << title << RESET << repeatChar(' ', right)
         << CYAN << "|" << RESET << "\n";
    printLine();
}

// Prints a progress bar like  [#####---------------] 2/5
void printProgress(int done, int total) {
    int barSize = 20;
    int filled = (done * barSize) / total;

    cout << "[" << GREEN << repeatChar('#', filled)
         << GRAY << repeatChar('-', barSize - filled) << RESET
         << "] " << done << "/" << total;
}

void waitForEnter() {
    cout << GRAY << "\n  Press Enter to continue..." << RESET;
    string dummy;
    getline(cin, dummy);
}

// Makes text lowercase so "Easy" and "easy" match
string toLower(string s) {
    for (int i = 0; i < (int)s.length(); i++)
        s[i] = tolower(s[i]);
    return s;
}

// Input validation 
int readInt(int low, int high) {
    int value;
    while (true) {
        cout << YELLOW << "  > " << RESET;
        if (cin >> value && value >= low && value <= high) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');   // clear rest of the line
            return value;
        }

        if (cin.eof())
            exit(0);                                              // input closed, stop the program

        cin.clear();                                              // reset error state
        cin.ignore(numeric_limits<streamsize>::max(), '\n');      // throw away bad input
        cout << RED << "  Please enter a number from " << low << " to " << high << "." << RESET << "\n";
    }
}



vector<Question> loadQuestions(const string& filename) {
    vector<Question> questions;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Could not open " << filename << "\n";
        return questions;
    }

    string line;
    getline(file, line);   

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();                      

        stringstream ss(line);                    
        string category, difficulty, text, a, b, c, d, correctText;

        getline(ss, category, ',');
        getline(ss, difficulty, ',');
        getline(ss, text, ',');
        getline(ss, a, ',');
        getline(ss, b, ',');
        getline(ss, c, ',');
        getline(ss, d, ',');
        getline(ss, correctText, ',');

        int correct = atoi(correctText.c_str());

        // skip blank rows and rows with a bad answer number
        if (text.empty() || correct < 1 || correct > 4)
            continue;

        Question q(category, difficulty, text, a, b, c, d, correct);
        questions.push_back(q);
    }
    return questions;
}

int loadHighScore() {
    ifstream file("highscore.txt");
    int best = 0;
    if (file.is_open())
        file >> best;
    return best;
}

void saveHighScore(int percent) {
    ofstream file("highscore.txt");
    file << percent;
}

vector<string> getCategories(const vector<Question>& questions) {
    vector<string> categories;
    for (int i = 0; i < (int)questions.size(); i++) {
        bool found = false;
        for (int j = 0; j < (int)categories.size(); j++)
            if (categories[j] == questions[i].getCategory())
                found = true;
        if (!found)
            categories.push_back(questions[i].getCategory());
    }
    return categories;
}



// Pick a category ("" means all) 
string chooseCategory(const vector<Question>& questions) {
    vector<string> categories = getCategories(questions);

    clearScreen();
    printTitle("CHOOSE A CATEGORY");
    cout << "\n";
    cout << "  " << YELLOW << "[1]" << RESET << " All categories\n";
    for (int i = 0; i < (int)categories.size(); i++)
        cout << "  " << YELLOW << "[" << (i + 2) << "]" << RESET << " " << categories[i] << "\n";
    cout << "\n";

    int choice = readInt(1, categories.size() + 1);
    if (choice == 1)
        return "";
    return categories[choice - 2];
}

// Pick a difficulty ("" means all)
string chooseDifficulty() {
    clearScreen();
    printTitle("CHOOSE A DIFFICULTY");
    cout << "\n";
    cout << "  " << YELLOW << "[1]" << RESET << " All levels\n";
    cout << "  " << YELLOW << "[2]" << RESET << " " << GREEN  << "Easy"   << RESET << "\n";
    cout << "  " << YELLOW << "[3]" << RESET << " " << YELLOW << "Medium" << RESET << "\n";
    cout << "  " << YELLOW << "[4]" << RESET << " " << RED    << "Hard"   << RESET << "\n";
    cout << "\n";

    switch (readInt(1, 4)) {
        case 2:  return "easy";
        case 3:  return "medium";
        case 4:  return "hard";
        default: return "";
    }
}

// Draw one question screen 
// picked = 0 means the player has not answered yet
void drawQuestion(const Question& q, int number, int total, int score, int picked) {
    clearScreen();
    printTitle("QUESTION " + to_string(number) + " OF " + to_string(total));

    cout << "\n  Progress: ";
    printProgress(number - 1, total);
    cout << "\n  Score: " << YELLOW << score << RESET
         << "     " << GRAY << q.getCategory() << " - " << q.getDifficulty() << RESET << "\n\n";

    cout << "  " << BOLD << q.getText() << RESET << "\n\n";

    for (int i = 0; i < 4; i++) {
        int num = i + 1;
        string choice = q.getChoice(i);

        if (picked == 0) {
            cout << "  " << YELLOW << "[" << num << "]" << RESET << " " << choice << "\n";
        }
        else if (num == q.getCorrect()) {
            cout << "  " << GREEN << "[" << num << "] " << choice << "  <- correct" << RESET << "\n";
        }
        else if (num == picked) {
            cout << "  " << RED << "[" << num << "] " << choice << "  <- your answer" << RESET << "\n";
        }
        else {
            cout << "  " << GRAY << "[" << num << "] " << choice << RESET << "\n";
        }
    }
    cout << "\n";
}

// Ask one question 
bool askQuestion(const Question& q, int number, int total, int score) {
    drawQuestion(q, number, total, score, 0);   // show it without answers marked
    int answer = readInt(1, 4);

    drawQuestion(q, number, total, score, answer);   // redraw with colors

    bool isCorrect = (answer == q.getCorrect());
    if (isCorrect)
        cout << "  " << GREEN << BOLD << "Correct!" << RESET << "\n";
    else
        cout << "  " << RED << BOLD << "Wrong!" << RESET << "\n";

    waitForEnter();
    return isCorrect;
}

//  Results screen 
void showResults(int score, int amount) {
    int percent = (score * 100) / amount;

    clearScreen();
    printTitle("RESULTS");
    cout << "\n  Final score: " << BOLD << score << "/" << amount << RESET
         << " (" << percent << "%)\n\n  ";
    printProgress(score, amount);
    cout << "\n\n  ";

    if (percent >= 90)      cout << GREEN  << "Excellent!";
    else if (percent >= 70) cout << GREEN  << "Great job!";
    else if (percent >= 50) cout << YELLOW << "Not bad!";
    else                    cout << RED    << "Keep practicing!";
    cout << RESET << "\n";

    if (percent > loadHighScore()) {
        saveHighScore(percent);
        cout << "\n  " << YELLOW << BOLD << "** NEW HIGH SCORE! **" << RESET << "\n";
    }

    waitForEnter();
}

// Play one round 
void playGame(const vector<Question>& all) {
    string category = chooseCategory(all);
    string difficulty = chooseDifficulty();

    // keep only the questions that match the chosen category and difficulty
    vector<Question> pool;
    for (int i = 0; i < (int)all.size(); i++) {
        bool categoryOk = (category == "" || all[i].getCategory() == category);
        bool difficultyOk = (difficulty == "" || toLower(all[i].getDifficulty()) == difficulty);

        if (categoryOk && difficultyOk)
            pool.push_back(all[i]);
    }

    if (pool.empty()) {
        cout << "\n  " << RED << "No questions match that choice. Try something else." << RESET << "\n";
        waitForEnter();
        return;
    }

    // the most questions allowed is 3, or fewer if there aren't enough
    int maxAmount = MAX_QUESTIONS;
    if ((int)pool.size() < maxAmount)
        maxAmount = pool.size();

    clearScreen();
    printTitle("HOW MANY QUESTIONS?");
    cout << "\n  Pick a number from 1 to " << maxAmount << ":\n\n";
    int amount = readInt(1, maxAmount);

    // shuffle the order so every game is different
    mt19937 rng(random_device{}());
    shuffle(pool.begin(), pool.end(), rng);

    int score = 0;
    for (int i = 0; i < amount; i++) {
        if (askQuestion(pool[i], i + 1, amount, score))
            score++;
    }

    showResults(score, amount);
}

// Main menu 
int main() {
    system("");   

    vector<Question> questions = loadQuestions("questions.csv");

    if (questions.empty()) {
        cout << "No questions loaded. Make sure questions.csv is in the same folder.\n";
        return 1;
    }

    bool running = true;
    while (running) {
        clearScreen();
        printTitle("RANDOM QUIZ GAME");
        cout << "\n";
        cout << "  " << YELLOW << "[1]" << RESET << " Play\n";
        cout << "  " << YELLOW << "[2]" << RESET << " Quit\n\n";
        cout << "  Best score: " << GREEN << loadHighScore() << "%" << RESET << "\n\n";

        switch (readInt(1, 2)) {
            case 1:
                playGame(questions);
                break;
            case 2:
                running = false;
                clearScreen();
                cout << "Thanks for playing!\n";
                break;
        }
    }
    return 0;
}

