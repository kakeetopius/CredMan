
# 🔐 CredMan

**CredMan** (Credential Manager) is a lightweight C-based tool for securely generating, storing, retrieving, and managing user credentials.  
Credentials are stored in a small sqlite3 database that is automatically encrypted in place by an sqlite3 fork - sqlcipher.  
It’s designed to provide a simple and minimal credential management layer for local or embedded applications.

---

## 🚀 Features
- Securely store and retrieve credentials
- Cryptographically secure random password generator.
- Simple command-line interface (CLI)
- Encrypted local storage
- Cross-platform C implementation
- Lightweight and minimal to no dependency design

---

## 🛠️ Build and Run

```bash
# Clone the repository
git clone https://github.com/kakeetopius/CredMan.git
cd CredMan

# Build
make cman

# Run
./cman help 
