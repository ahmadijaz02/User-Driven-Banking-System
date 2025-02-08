# User-Driven Banking System  
User-Driven Banking System is a multi-threaded banking application developed in C with a console-driven interface. It enables users to create accounts, perform transactions, and view balances while ensuring efficient and secure banking operations. 
**Features:** 
Account Management (create, update, manage accounts), Transactions (deposit, withdraw, check balance), Multi-threading (handles concurrent transactions using mutex locks), Process Scheduling (manages transaction execution order for efficiency), Transaction Logging (records all transactions for tracking), LRU-based Memory Management (optimizes memory usage). 
**Installation:** 
Clone the repository:
`git clone https://github.com/yourusername/user-driven-banking-system.git && cd user-driven-banking-system`,
Compile:
`gcc banking_system.c -o banking_system -pthread`,
Run: `./banking_system`.
**Usage:**
Follow on-screen prompts to create an account and perform actions like deposit, withdraw, or check balance, with secure processing and logging.
**Technologies Used:** 
C (programming), POSIX Threads (concurrency), LRU Caching (optimized memory management).
**Future Enhancements:** 
GUI implementation, database integration for persistent storage, role-based access control for security. **License:** This project is licensed under the MIT License.  
