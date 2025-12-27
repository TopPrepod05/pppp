#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>

using namespace std;

string generate_secret(int length) {
	// первая цифра не '0', все цифры уникальны
	string digits = "0123456789";
	mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
	uniform_int_distribution<int> dist;
	string secret;
	// выбрать первую цифру из '1'..'9'
	vector<char> pool(digits.begin()+1, digits.end());
	shuffle(pool.begin(), pool.end(), rng);
	secret.push_back(pool.back()); pool.pop_back();
	// добавить остальные уникальные цифры (можно взять '0' теперь)
	pool.insert(pool.end(), digits[0]);
	shuffle(pool.begin(), pool.end(), rng);
	for (int i = 1; i < length; ++i) {
		secret.push_back(pool.back());
		pool.pop_back();
	}
	return secret;
}

bool valid_guess(const string& g, int length, string& reason) {
	if (g == "hint") return true;
	if ((int)g.size() != length) { reason = "Ожидается " + to_string(length) + " цифр."; return false; }
	for (char c : g) if (!isdigit((unsigned char)c)) { reason = "Только цифры разрешены."; return false; }
	if (g[0] == '0') { reason = "Первая цифра не должна быть 0."; return false; }
	set<char> s(g.begin(), g.end());
	if ((int)s.size() != length) { reason = "Цифры не должны повторяться."; return false; }
	return true;
}

int count_bulls(const string& secret, const string& guess) {
	int b = 0;
	for (size_t i = 0; i < secret.size() && i < guess.size(); ++i)
		if (secret[i] == guess[i]) ++b;
	return b;
}

int count_cows(const string& secret, const string& guess) {
	int common = 0;
	for (char d = '0'; d <= '9'; ++d) {
		int cs = count(secret.begin(), secret.end(), d);
		int cg = count(guess.begin(), guess.end(), d);
		common += min(cs, cg);
	}
	return common - count_bulls(secret, guess);
}

pair<int,int> choose_difficulty() {
	while (true) {
		cout << "Выберите сложность:\n1) Легко — 4 цифры, 10 попыток\n2) Нормально — 5 цифр, 12 попыток\n3) Сложно — 6 цифр, 15 попыток\nВаш выбор (1/2/3): ";
		string ch; if (!getline(cin, ch)) return {4,10};
		if (ch == "1") return {4,10};
		if (ch == "2") return {5,12};
		if (ch == "3") return {6,15};
		cout << "Неправильный выбор. Введите 1, 2 или 3.\n";
	}
}

pair<int,char> reveal_hint(const string& secret, set<int>& revealed) {
	vector<int> idx;
	for (int i = 0; i < (int)secret.size(); ++i) if (!revealed.count(i)) idx.push_back(i);
	if (idx.empty()) return {-1, 0};
	mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
	uniform_int_distribution<int> d(0, (int)idx.size()-1);
	int i = idx[d(rng)];
	revealed.insert(i);
	return {i, secret[i]};
}

pair<bool,int> play_once() {
	auto [length, max_attempts] = choose_difficulty();
	string secret = generate_secret(length);
	int attempts = 0;
	bool hint_used = false;
	set<int> revealed;
	// cout << "DEBUG secret: " << secret << "\n";
	while (attempts < max_attempts) {
		cout << "Попытка " << attempts+1 << "/" << max_attempts << ". Введите число или \"hint\": ";
		string guess; if (!getline(cin, guess)) return {false, attempts};
		string reason;
		if (!valid_guess(guess, length, reason)) { cout << "Ошибка: " << reason << "\n"; continue; }
		if (guess == "hint") {
			if (hint_used) { cout << "Подсказка уже использована.\n"; continue; }
			auto pr = reveal_hint(secret, revealed);
			if (pr.first == -1) cout << "Нет доступных позиций для подсказки.\n";
			else cout << "Подсказка: позиция " << pr.first+1 << " = " << pr.second << "\n";
			hint_used = true;
			continue;
		}
		++attempts;
		int bulls = count_bulls(secret, guess);
		int cows = count_cows(secret, guess);
		if (bulls == length) {
			cout << "Поздравляем! Вы угадали " << secret << " за " << attempts << " попыток.\n";
			return {true, attempts};
		}
		cout << "Быки: " << bulls << ", Коровы: " << cows << "\n";
	}
	cout << "Попытки исчерпаны. Загаданное число было: " << secret << "\n";
	return {false, attempts};
}

int main() {
	int total_games = 0, wins = 0, total_attempts = 0;
	cout << "Игра «Быки и коровы»\n";
	while (true) {
		++total_games;
		auto [win, attempts] = play_once();
		if (win) ++wins;
		total_attempts += attempts;
		cout << "Статистика: сыграно " << total_games << ", побед " << wins
			 << ", поражений " << total_games - wins
			 << ", средние попытки на игру " << (double)total_attempts / total_games << "\n";
		cout << "Сыграть ещё раз? (y/n): ";
		string ans; if (!getline(cin, ans)) break;
		if (ans != "y" && ans != "Y") break;
	}
	cout << "Спасибо за игру.\n";
	return 0;
}