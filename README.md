# User-Driven Banking System

The User-Driven Banking System is a multi-threaded banking application developed in C with a console-driven interface. It enables users to create accounts, perform transactions, and view balances while ensuring efficient and secure banking operations.

## Features
- **Account Management:** Create, update, and manage accounts.
- **Transactions:** Deposit, withdraw, and check balances.
- **Multi-threading:** Handles concurrent transactions using mutex locks.
- **Process Scheduling:** Manages transaction execution order for efficiency.
- **Transaction Logging:** Records all transactions for tracking.
- **LRU-based Memory Management:** Optimizes memory usage.

## Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/user-driven-banking-system.git && cd user-driven-banking-system
   ```
2. Compile the application:
   ```bash
   gcc banking_system.c -o banking_system -pthread
   ```
3. Run the application:
   ```bash
   ./banking_system
   ```

## Usage
Follow the on-screen prompts to create an account and perform actions like deposit, withdraw, or check balance, with secure processing and logging.

## Technologies Used
- **C:** Programming language.
- **POSIX Threads:** For concurrency.
- **LRU Caching:** For optimized memory management.

## Future Enhancements
- GUI implementation
- Database integration for persistent storage
- Role-based access control for enhanced security

## License
This project is licensed under the MIT License.
