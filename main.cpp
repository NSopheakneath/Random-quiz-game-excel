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

const int MAX_QUESTIONS = 3;   

// Class: one quiz question
class Question {
private:
    string category;
    string difficulty;
    string text;
    string choices[4];   // array of 4 answers
    int correct;         // 1-4

public:
    // Constructor: 
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

//Helper: make text lowercase so "Easy" and "easy" match
string toLower(string s) {
    for (int i = 0; i < (int)s.length(); i++)
        s[i] = tolower(s[i]);
    return s;
}

// Input validation 
int readInt(int low, int high) {
    int value;
    while (true) {
        cout << "> ";
        if (cin >> value && value >= low && value <= high)
            return value;

        cin.clear();                                          // reset error state
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // throw away bad input
        cout << "Please enter a number from " << low << " to " << high << ".\n";
    }
}

// File I/O: load questions from CSV 
vector<Question> loadQuestions(const string& filename) {
    vector<Question> questions;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Could not open " << filename << "\n";
        return questions;
    }

    string line;
    getline(file, line);   // skip the header row

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();                      // Windows line ending

        stringstream ss(line);                    // lets us split the line by commas
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

//  Build the category list from the data 
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

// Menu: pick a category ("" means all) 
string chooseCategory(const vector<Question>& questions) {
    vector<string> categories = getCategories(questions);

    cout << "\nChoose a category:\n";
    cout << "  1) All\n";
    for (int i = 0; i < (int)categories.size(); i++)
        cout << "  " << (i + 2) << ") " << categories[i] << "\n";

    int choice = readInt(1, categories.size() + 1);
    if (choice == 1)
        return "";
    return categories[choice - 2];
}

//  Menu: pick a difficulty ("" means all) 
string chooseDifficulty() {
    cout << "\nChoose a difficulty:\n";
    cout << "  1) All\n  2) Easy\n  3) Medium\n  4) Hard\n";

    switch (readInt(1, 4)) {
        case 2:  return "easy";
        case 3:  return "medium";
        case 4:  return "hard";
        default: return "";
    }
}

// Ask one question 
bool askQuestion(const Question& q, int number, int total) {
    cout << "\nQuestion " << number << "/" << total
         << "  [" << q.getCategory() << " - " << q.getDifficulty() << "]\n";
    cout << q.getText() << "\n\n";

    for (int i = 0; i < 4; i++)
        cout << "  " << (i + 1) << ") " << q.getChoice(i) << "\n";
    cout << "\n";

    int answer = readInt(1, 4);

    if (answer == q.getCorrect()) {
        cout << "Correct!\n";
        return true;
    }
    cout << "Wrong! The answer was: " << q.getChoice(q.getCorrect() - 1) << "\n";
    return false;
}

// High score file 
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
        cout << "\nNo questions match that choice. Try something else.\n";
        return;
    }

    // the most questions allowed is 5, or fewer if there aren't enough
    int maxAmount = MAX_QUESTIONS;
    if ((int)pool.size() < maxAmount)
        maxAmount = pool.size();

    cout << "\nHow many questions? (1-" << maxAmount << ")\n";
    int amount = readInt(1, maxAmount);

    // shuffle the order so every game is different
    mt19937 rng(random_device{}());
    shuffle(pool.begin(), pool.end(), rng);

    int score = 0;
    for (int i = 0; i < amount; i++) {
        if (askQuestion(pool[i], i + 1, amount))
            score++;
    }

    int percent = (score * 100) / amount;

    cout << "\n==============================\n";
    cout << " Final score: " << score << "/" << amount << " (" << percent << "%)\n";

    if (percent >= 90)      cout << " Excellent!\n";
    else if (percent >= 70) cout << " Great job!\n";
    else if (percent >= 50) cout << " Not bad!\n";
    else                    cout << " Keep practicing!\n";

    if (percent > loadHighScore()) {
        saveHighScore(percent);
        cout << " New high score!\n";
    }
    cout << "==============================\n";
}

// Main menu
int main() {
    vector<Question> questions = loadQuestions("questions.csv");

    if (questions.empty()) {
        cout << "No questions loaded. Make sure questions.csv is in the same folder.\n";
        return 1;
    }

    bool running = true;
    while (running) {
        cout << "\n==============================\n";
        cout << "        C++ QUIZ GAME\n";
        cout << "==============================\n";
        cout << "  1) Play\n";
        cout << "  2) View high score\n";
        cout << "  3) Quit\n";

        switch (readInt(1, 3)) {
            case 1:
                playGame(questions);
                break;
            case 2:
                cout << "\nBest score: " << loadHighScore() << "%\n";
                break;
            case 3:
                running = false;
                cout << "Thanks for playing!\n";
                break;
        }
    }
    return 0;
}
