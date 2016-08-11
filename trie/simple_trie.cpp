/*
  Simple trie data structure to play with c++11.
*/

#include <boost/algorithm/string.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

namespace sc = std::chrono;

typedef std::set<std::string> wordset_t;

class trie_t;

typedef std::map<char, trie_t *> child_map_t;

class trie_t {
public:
  trie_t(bool end = false) : _size(0), _isEnd(end) {}
  ~trie_t() {
    for (auto &it : _children) {
      delete it.second;
    }
  }

  void addWord(std::string word) {
    if (word.length() > 0) {
      ++_size;
      std::string subword = word.substr(1, word.size() - 1);
      if (_children[word[0]]) {
        _children[word[0]]->addWord(subword);
      } else {
        trie_t *tmp = new trie_t(word.size() == 1);
        tmp->addWord(subword);
        _children[word[0]] = tmp;
      }
    }
  }

  bool isPrefix(std::string pref) const {
    if (pref.length() == 0) {
      return true;
    }
    if (_children.find(pref[0]) != _children.end()) {
      return _children.find(pref[0])->second->isPrefix(
          pref.substr(1, pref.size() - 1));
    }
    return false;
  }

  bool isWord(std::string word) const {
    if (word.length() == 0) {
      return _isEnd;
    }
    std::string cursub;
    trie_t const *tmp = this;
    cursub = word;

    while (cursub.length() > 0) {
      if (tmp->_children.find(cursub[0]) != tmp->_children.end()) {
        tmp = tmp->_children.find(cursub[0])->second;
        cursub = cursub.substr(1, cursub.size() - 1);
      } else {
        return false;
      }
    }
    return tmp->isWordEnd();
  }

  size_t size() { return _size; }
  void getWords(wordset_t &words, std::string wordSoFar = "") const {
    if (_isEnd) {
      words.insert(wordSoFar);
    }
    for (const auto &it : _children) {
      std::string tmp = wordSoFar + std::string(1, it.first);
      if (it.second && it.second->isWordEnd()) {
        words.insert(tmp);
      }
      it.second->getWords(words, tmp);
    }
  }

  void getWordsStartingWith(std::string prefix, wordset_t &words,
                            std::string wordSoFar = "") const {
    if (prefix.size() == 0) {
      getWords(words, wordSoFar);
      return;
    }
    std::string subword = prefix.substr(1, prefix.size() - 1);
    if (_children.find(prefix[0]) != _children.end()) {
      trie_t *tmp = _children.find(prefix[0])->second;
      std::string nwsf = wordSoFar + std::string(1, prefix[0]);
      tmp->getWordsStartingWith(subword, words, nwsf);
    }
  }

private:
  bool isWordEnd() const { return _isEnd; }

private:
  child_map_t _children;
  size_t _size;
  bool _isEnd;
};

void buildDictionaryTrie(trie_t &ot, std::string fname) {
  // const std::regex wordRx("[a-z]+");

  std::ifstream inf(fname);
  std::string curWord;
  while (inf >> curWord) {
    boost::to_lower(curWord);
    if (curWord.find_first_not_of("abcdefghijklmnopqrstuvwxyz") ==
        std::string::npos) {

      // if (std::regex_match(curWord, wordRx)) {
      ot.addWord(curWord);
    }
  }
}

sc::time_point<sc::steady_clock> tstamp() { return sc::steady_clock::now(); }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "No word file given!\n";
    return 1;
  }

  std::string wordfile = argv[1];
  trie_t theTrie;
  std::cout << "Building trie...\n";
  auto start = tstamp();
  buildDictionaryTrie(theTrie, wordfile);
  auto end = tstamp();
  auto diff = end - start;
  std::cout << "Building trie took "
            << sc::duration<double, std::milli>(diff).count() << " ms\n";

  std::cout << "The trie has " << theTrie.size() << " words.\n";

  // TODO: Find out why these sizes can be different - probably a bug
  wordset_t wset;
  theTrie.getWords(wset);
  std::cout << "The word set has " << wset.size() << " words.\n";

  std::string prefix;
  while (std::cin >> prefix) {

    start = tstamp();
    bool isp = theTrie.isPrefix(prefix);
    end = tstamp();
    diff = end - start;
    std::cout << "Took " << sc::duration<double, std::milli>(diff).count()
              << " to find that \"" << prefix << "\" "
              << (isp ? "is" : "is not") << " a prefix\n";

    wset.clear();
    if (isp) {
      start = tstamp();
      theTrie.getWordsStartingWith(prefix, wset);
      end = tstamp();
      diff = end - start;
      std::cout << "Took " << sc::duration<double, std::milli>(diff).count()
                << " to find " << wset.size() << " words starting with \""
                << prefix << "\":\n";

      for (const auto &wrd : wset) {
        std::cout << "\t" << wrd << "\n";
      }
    }
  }
  return 0;
}
