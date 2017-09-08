#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <string>
#include <vector>
#include <tr1/functional>

#include <unistd.h>

using namespace std;

unsigned int generateID() {
  static unsigned int idMaker {0};
  return ++idMaker;
}

/*!
 * 用户文件目录
 * User File Directory
 */
struct UFD {
  enum FileAttribute {
    READ = 0x1,
    READWRITE = 0x1 << 1,
  };

  bool canRead() const { return m_fileAttribute & READ;}
  bool canWrite() const { return m_fileAttribute & READWRITE; }
  bool canReadWrite() const { return canRead() && canWrite(); }

  unsigned int m_id {0};
  string m_filename;
  int m_fileAttribute {0};
  int m_length {0};
  int m_physicalAddress {0};
};

/*!
 * 用户打开文件表
 * User Open File
 */
struct UOF {
  enum OpenState {
    OPEN = 0x1,
    CREATE = 0x1 << 1,
  };

  bool isOpen() const { return m_openState & OPEN; }
  bool isCreate() const { return m_openState & CREATE;}

  unsigned int m_id {0};
  int m_readPtr {-1};
  int m_writePtr {-1};
  int m_openState {0};
};


/*!
 * 主文件目录
 * Main File Directory
 */
struct MFD {
  ~MFD() { 
    if(m_ufd) {
      delete m_ufd;
    }
  }

  string m_username;
  UFD *m_ufd {nullptr};
};

/*!
 * 文件资源管理器
 */
class FileManager {
public:
  FileManager() {}
  ~FileManager() { clean(); }

  MFD* selectUser(const string& username) const;
  MFD* currentUser() {
    const FileManager* fm = const_cast<const FileManager*>(this);
    return const_cast<MFD*>(fm->currentUser());
  }

  const MFD* currentUser() const {
    if(m_currentuser<0 || m_currentuser>= m_users.size())
      return nullptr;

    return m_users[m_currentuser];
  }

  bool runCommand(const string& cmd, string & errorString, char *& result);
private:
  void clean();
  //文件操作
private:
  bool create(const string& filename, int length, int attribute);
  bool open(const string& filename, const string& optType);
  bool write(const string& filename, const string& records);
private:
  //多用户多文件管理
  vector<MFD*> m_users;
  //当前用户索引值
  int m_currentuser {-1};
};

MFD* FileManager::selectUser(const string& username) const{
  vector<MFD*>::iterator it =
    find_if(m_users.begin(), m_users.end(), [=](MFD &*mfd) { if(!mfd) return false; return username==mfd->m_username;});
  if(it == m_users.end())
    return nullptr;

  m_currentuser = it - m_users.begin();
  return *it;
}

bool FileManager::runCommand(const string& cmd, string& errorString, char *& result) {
  cout<<"run cmd:"<<cmd<<endl;
  return true;
}

void FileManager::clean() {
  vector<MFD*>::iterator it = m_users.begin();
  while(it != m_users.end()) {
    delete *it;
    it = m_users.erase(it);
  }
}

int main(int argc, char *argv[]) {
  return 0;
}



