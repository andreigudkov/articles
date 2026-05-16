#include <filesystem>
#include <cstdio>
#include <string>
#include <fstream>
#include <regex>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

const std::regex re("https?://[^\\s]+",
  std::regex_constants::ECMAScript|
  std::regex_constants::icase|
  std::regex_constants::optimize);

bool MaybeGarbage(char c) {
  return !(std::isalnum(static_cast<unsigned char>(c)) || (c == '/'));
}


/*************** ReportUrls ****************/

typedef void(*Callback)(const std::string&);

void ReportUrls(const std::string& path, Callback callback) {
  // level1: iterate over the files
  for (auto& fse : std::filesystem::directory_iterator(path)) {
    std::ifstream ifs(fse.path().c_str());
    if (!ifs.is_open()) {
      throw std::runtime_error("Failed to open file");
    }

    // level2: iterate over the lines of the jsonl file
    std::string line;
    while (std::getline(ifs, line)) {
      auto jcomments = json::parse(line)["comments"];

      // level3: iterate over the array of comments
      for (size_t i = 0; i < jcomments.size(); i++) {
        std::string comment = jcomments[i].get<std::string>();

        // level4: iterate over regexp matches
        for (auto reit = std::sregex_iterator(comment.begin(), comment.end(), re);
             reit != std::sregex_iterator(); reit++) {
          std::string url = reit->str() + ',';

          // level5: iterate over the prefixes of the candidate URL
          while (MaybeGarbage(url.back())) {
            url.pop_back();
            callback(url);
          }
        }
      }
    }
  }
}

/*************** ClassicFSM ****************/

class ClassicFSM {
public:
  enum class State : uint8_t {
    GET_FILE = 1,
    GET_LINE = 2,
    GET_COMMENT = 3,
    GET_MATCH = 4,
    GET_PREFIX = 5
  };

  ClassicFSM(const std::string& path) :
    dirit_(path),
    jpos_(0)
  {
  }

  const std::string* Next() {
    State state = State::GET_PREFIX;
    while (true) {
      switch (state) {

        // level1: iterate over the files
        case State::GET_FILE: {
          if (dirit_ != std::filesystem::directory_iterator()) {
            ifs_.open(dirit_->path().c_str());
            dirit_++;
            if (!ifs_.is_open()) {
              throw std::runtime_error("Failed to open file");
            }
            state = State::GET_LINE;
          } else {
            return nullptr;
          }
          continue;
        }

        // level2: iterate over the lines of the jsonl files
        case State::GET_LINE: {
          std::string line;
          if (std::getline(ifs_, line)) {
            jcomments_ = json::parse(line)["comments"];
            jpos_ = 0;
            state = State::GET_COMMENT;
          } else {
            if (ifs_.is_open()) {
              ifs_.close();
            }
            state = State::GET_FILE;
          }
          continue;
        }

        // level3: iterate over the array of comments
        case State::GET_COMMENT: {
          if (jpos_ < jcomments_.size()) {
            comment_ = jcomments_[jpos_].get<std::string>();
            jpos_++;
            reit_ = std::sregex_iterator(comment_.begin(), comment_.end(), re);
            state = State::GET_MATCH;
          } else {
            state = State::GET_LINE;
          }
          continue;
        }

        // level4: iterate over regexp matches
        case State::GET_MATCH: {
          if (reit_ != std::sregex_iterator()) {
            match_ = reit_->str() + ',';
            reit_++;
            state = State::GET_PREFIX;
          } else {
            state = State::GET_COMMENT;
          }
          continue;
        }

        // level5: iterate over the prefixes of the match
        case State::GET_PREFIX: {
          if (!match_.empty() && MaybeGarbage(match_.back())) {
            match_.pop_back();
            return &match_;
          } else {
            state = State::GET_MATCH;
          }
          continue;
        }

        default: {
          throw std::runtime_error("unreachable");
        }
      } // switch
    } // while
  }

private:
  std::filesystem::directory_iterator dirit_; // GET_FILE
  std::ifstream ifs_;         // GET_LINE
  json jcomments_;            // GET_COMMENT: comments array
  size_t jpos_;               // GET_COMMENT: position inside jcomments_
  std::string comment_;       // GET_MATCH: buffer for reit_
  std::sregex_iterator reit_; // GET_MATCH
  std::string match_;         // GET_URL
};


/*************** MultifuncFSM ****************/

class MultifuncFSM {
public:
  MultifuncFSM(const std::string& path) :
    dirit_(path),
    jpos_(0)
  {
  }

  const std::string* Next() {
    return GetPrefix();
  }

private:
  bool GetFile() {
    while (true) {
      if (dirit_ != std::filesystem::directory_iterator()) {
        ifs_.open(dirit_->path().c_str());
        dirit_++;
        if (!ifs_.is_open()) {
          throw std::runtime_error("Failed to open file");
        }
        return true;
      } else {
        return false;
      }
    }
  }

  bool GetLine() {
    while (true) {
      std::string line;
      if (std::getline(ifs_, line)) {
        jcomments_ = json::parse(line)["comments"];
        jpos_ = 0;
        return true;
      } else {
        if (ifs_.is_open()) {
          ifs_.close();
        }
        if (!GetFile()) {
          return false;
        }
      }
    }
  }

  bool GetComment() {
    while (true) {
      if (jpos_ < jcomments_.size()) {
        comment_ = jcomments_[jpos_++].get<std::string>();
        reit_ = std::sregex_iterator(comment_.begin(), comment_.end(), re);
        return true;
      } else {
        if (!GetLine()) {
          return false;
        }
      }
    }
  }

  bool GetMatch() {
    while (true) {
      if (reit_ != std::sregex_iterator()) {
        match_ = reit_->str() + ",";
        reit_++;
        return true;
      } else {
        if (!GetComment()) {
          return false;
        }
      }
    }
  }

  const std::string* GetPrefix() {
    while (true) {
      if (!match_.empty() && MaybeGarbage(match_.back())) {
        match_.pop_back();
        return &match_;
      } else {
        if (!GetMatch()) {
          return nullptr;
        }
      }
    }
  }

private:
  std::filesystem::directory_iterator dirit_;
  std::ifstream ifs_;
  json jcomments_;
  size_t jpos_;
  std::string comment_;
  std::sregex_iterator reit_;
  std::string match_;
};

/*************** TopDown ****************/

class TopDown {
public:
  TopDown(const std::string& path) :
    dirit_(new std::filesystem::directory_iterator(path)),
    jpos_(0)
  {}

  const std::string* Next() {
    while (true) {
      // ensure that we have iterator over the files
      if (!dirit_) {
        return nullptr;
      }

      // ensure that we have iterator over the lines
      if (!ifs_) {
        if (*dirit_ != std::filesystem::directory_iterator()) {
          ifs_.open((*dirit_)->path().c_str());
          (*dirit_)++;
          if (!ifs_.is_open()) {
            throw std::runtime_error("failed to open file");
          }
        } else {
          dirit_.reset();
          continue;
        }
      }

      // ensure that we have iterator over comments
      if (jcomments_.is_null()) {
        std::string line;
        if (std::getline(ifs_, line)) {
          jcomments_ = json::parse(line)["comments"];
          jpos_ = 0;
        } else {
          ifs_.close();
          continue;
        }
      }

      // ensure that we have iterator over matches of a single comment
      if (!reit_) {
        if (jpos_ < jcomments_.size()) {
          comment_ = jcomments_[jpos_++].get<std::string>();
          reit_.reset(new std::sregex_iterator(comment_.begin(), comment_.end(), re));
        } else {
          jcomments_ = json();
          continue;
        }
      }

      // ensure that we have iterator over prefixes of a single match
      if (!match_) {
        if (*reit_ != std::sregex_iterator()) {
          match_.reset(new std::string((*reit_)->str() + ","));
          (*reit_)++;
        } else {
          reit_.reset();
          continue;
        }
      }

      // nothing to ensure anymore -- try to read the bottom-most iterator
      if (MaybeGarbage(match_->back())) {
        match_->pop_back();
        return match_.get();
      } else {
        match_.reset();
      }
    }
  }

private:
  std::unique_ptr<std::filesystem::directory_iterator> dirit_;

  // nullability is modeled with closed stream 
  std::ifstream ifs_;

  // nullability is modeled with json null
  json jcomments_;
  size_t jpos_;
  
  std::unique_ptr<std::sregex_iterator> reit_;
  std::string comment_; // buffer for regex iterator

  std::unique_ptr<std::string> match_;
};



int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  const std::string path("./books");

  //ReportUrls(path, [](const std::string& url){
  //  std::printf("|%s|\n", url.c_str());
  //});

  //ClassicFSM it(path);
  //MultifuncFSM it(path);
  TopDown it(path);
  while (true) {
    const std::string* url = it.Next();
    if (url) {
      std::printf("|%s|\n", url->c_str());
    } else {
      break;
    }
  }

  return 0;
}

