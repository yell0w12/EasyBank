# Banking Transaction Simulator

A C++ banking system that simulates account management, transactions, and financial queries with IP-based authentication.

## Compilation & Usage

```bash
# Compile
g++ main.cpp -o bank

# Run
./bank -v -f <registration_file.txt> < <commands_file.txt>
```

### Command Line Options
- `-v` or `--verbose`: Enable verbose output
- `-f <file>` or `--file <file>`: Specify registration file (required)
- `-h` or `--help`: Show help

## Program Overview

The simulator processes user accounts with PIN authentication, IP tracking, and scheduled transactions with dynamic fee calculation (1% of amount, min $10, max $450, with discounts for loyal customers).

## Commands

### Account Operations
- `l <user_id> <pin> <ip_address>` - Login user
- `o <user_id> <ip_address>` - Logout user  
- `b <user_id> <ip_address>` - Check balance

### Transactions
- `p <timestamp> <ip> <sender> <recipient> <amount> <execution_date> <fee_type>`
  - `fee_type`: `o` (sender pays) or `s` (shared between sender/recipient)
  - Execution date must be within 3 days of placement

### Queries (prefix with `$$$`)
- `l <start_time> <end_time>` - List transactions in time range
- `r <start_time> <end_time>` - Calculate bank revenue  
- `h <user_id>` - View transaction history (last 10 incoming/outgoing)
- `s <date>` - Daily summary

### Comments
- `# <text>` - Comment line (ignored)

## File Format

**Registration File:** `<timestamp>|<user_id>|<pin>|<balance>`

**Time Format:** `YY:MM:DD:HH:MM:SS` (e.g., `00:00:01:12:30:45`)

## Example

```bash
./bank -v -f test-10-reg.txt < test-10-commands.txt
```

