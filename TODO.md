# Things to implement or change.
1. Database Change.
    - Remove files for connecting to mysql database and use sqlite instead.
2. Security.
    - Encrypt the on disk file used by sqlite3 using openssl or the crypto based sqlite3 C library instead.
3. Server.
    - Later after the base is stable add a client server modal such that credential file can be setup on a single computer and can be 
      accessed securely by other devices on a network.
4. Database Access Api.
    - Create a generic module to handle access to the DB for various things like:
        - Get all credentials.
        - Get a particular credential
        - Add a credential.
        - Remove a credential.
        - Update/change credential details
    - These functions from this API should work with the already existing structures from the account.h file.
