#include <getopt.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using namespace std;


struct Options{
    bool verbose = false;
    string reg_file; 
};
void getOptions(int argc, char **argv, Options &options) {

    opterr = static_cast<int>(false);  // Let us handle all error output for command line options
    int choice;
    int index = 0;

    option longOptions[] = {
        {"verbose", no_argument, nullptr, 'v' },
        {"file", required_argument, nullptr, 'f'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, '\0'},
    }; 
    
    while ((choice = getopt_long(argc, argv, "f:hv", static_cast<option *>(longOptions), &index)) != -1) {
        switch (choice) {
        case 'h':
            //fix help printer function

        case 'v': 
            options.verbose = true; 
            break;
        
        case 'f': {  
            string arg { optarg };
            options.reg_file = arg;
            break;
        }  
        default:
            cerr << "INVALID OUTPUT MODE SPECIFIED" << endl;
            exit(1);
            break;

        }  // switch ..choice
    }  // while

}  // getOptions()


class Bank{

private:

    struct Transaction {
        int id;
        string sender;
        string recipient;
        uint32_t amount;
        uint64_t execution_date;
        uint32_t fee;
        char o_or_s;

        bool operator<(Transaction b){
            if (execution_date == b.execution_date)
            {
                return id < b.id;
            }
            return execution_date < b.execution_date;
        }
    };

    struct User{
        string id;
        string pin;
        uint64_t time;
        uint32_t balance;
        unordered_set<string> IPs;
        vector<Transaction> incoming;
        vector<Transaction> outgoing;
    };

    struct Transaction_Comp{

        bool operator()(Transaction a, Transaction b){
            if (a.execution_date != b.execution_date)
            {
                return b.execution_date < a.execution_date;
            } else
            {
                return b.id < a.id;
            }
        }
    };

    // hash table of users
    unordered_map<string, User> users;

    //priority queue of transactions
    priority_queue<Transaction, vector<Transaction>, Transaction_Comp> transaction_queue;

    //vector of transactions
    vector<Transaction> transaction_vec;

    string reg_file;

    bool verbose;

    uint64_t current_timestamp;
    string current_time;


    uint64_t get_time_as_int(string timestamp){
        char bad = ':';
        timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), bad), timestamp.end());
        uint64_t time = stoll(timestamp);
        return time;
    }

    string get_uint_as_string(uint64_t time){
        string time_string = to_string(time);
        u_int64_t gap = 12 - time_string.length();
        time_string.insert(0, gap, '0');

        time_string.insert(2, 1, ':');
        time_string.insert(5, 1, ':');
        time_string.insert(8, 1, ':');
        time_string.insert(11, 1, ':');
        time_string.insert(14, 1, ':');
        return time_string;
    }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////                                   //////////////////////////////////////////////////////////
///////////////////////////////        C  O  M  M  A  N  D S        /////////////////////////////////////////////////////////
//////////////////////////////                                     ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void login_func(string ip, string user_id, string pin){

        auto it = users.find(user_id);
        if (it != users.end())
        {
            if (pin == users[user_id].pin)
            {
                users[user_id].IPs.insert(ip);
                if(verbose){
                    cout << "USER " << user_id << " LOGGED IN.\n";
                }
            }
            else
            {
                if (verbose)
                {
                    cout << "LOGIN FAILED FOR " << user_id << ".\n";
                }
            }
            
        } else
        {
            if (verbose)
            {
                cout << "LOGIN FAILED FOR " << user_id << ".\n";
            }
        }
    }

    void logout_func(string &ip, string &user_id){
        auto it = users.find(user_id);
        if (it != users.end())
        {
            if (!users[user_id].IPs.empty())
            {
                auto it2 = users[user_id].IPs.find(ip);
                if (it2 != users[user_id].IPs.end())
                {
                    users[user_id].IPs.erase(ip);
                    if (verbose)
                    {
                        cout << "USER " << user_id << " LOGGED OUT.\n";
                    }
                }
                else
                {
                    // IP not found - no log out for this ip 
                    if (verbose)
                    {
                        cout << "LOGOUT FAILED FOR " << user_id << ".\n";
                    }
                    
                }
                
            }
            else
            {
                // user has no active sessions
                if (verbose)
                {
                    cout << "LOGOUT FAILED FOR " << user_id << ".\n";
                }
            }
            
        } else
        {
            // user not found in hash table
            if (verbose)
            {
                cout << "LOGOUT FAILED FOR " << user_id << ".\n";
            }
        }
    }


    /**
     * The purpose of this function is to calculate a given user's balance after validating that they are logged in
     */
    void balance_func(string &user_id, string &ip){
        
        auto it = users.find(user_id);
        if (it != users.end())
        {
         //good 
            if (users[user_id].IPs.empty())
            {
                if (verbose)
                {
                    cout << "ACCOUNT " << user_id << " IS NOT LOGGED IN.\n";
                }
                return;
            }
              
            auto it2 = users[user_id].IPs.find(ip);
            if (it2 != users[user_id].IPs.end())
            {
                if (current_timestamp > 0)
                {
                    cout << "AS OF " << current_timestamp << ", " << user_id << " " 
                        << "HAS A BALANCE OF $" << users[user_id].balance << ".\n";
                    
                }
                else
                {
                    //call convert uint64t to string
                    // no place transactions have come in yet. use time from user creation date
                    cout << "AS OF " << users[user_id].time << ", " << user_id << " " 
                        << "HAS A BALANCE OF $" << users[user_id].balance << ".\n";
                }
                
            }
            else
            {
                if (verbose)
                {
                    cout << "FRAUDULENT TRANSACTION DETECTED, ABORTING REQUEST.\n";
                }
            }
            
        }
        else
        {
            if (verbose)
            {
                cout << "ACCOUNT " << user_id << " DOES NOT EXIST.\n";
            }
            
            
        }
        
    }

    // This function processes pending transactions
    void process(uint64_t &time_stamp_n){

        while ((!transaction_queue.empty())&&(transaction_queue.top().execution_date <= time_stamp_n))
        {
            uint32_t sender_fee = transaction_queue.top().fee;
            uint32_t recipient_fee = 0;
            if (transaction_queue.top().o_or_s == 's')
            {
                // checking if the transaction fee is an odd or even number. Because fees are dvided and two and then shared,
                // for an odd fee 
                if ((transaction_queue.top().fee % 2) != 0)
                {   
                    recipient_fee = transaction_queue.top().fee / 2;
                    sender_fee = transaction_queue.top().fee - recipient_fee;
                }
                else
                {
                    sender_fee = transaction_queue.top().fee / 2;
                    recipient_fee = transaction_queue.top().fee /2;
                }
                
                if (users[transaction_queue.top().recipient].balance < recipient_fee)
                {
                    //recip doesn't have enough money to cover the shared transaction fee
                    if (verbose)
                    {
                        cout << "INSUFFICIENT FUNDS TO PROCESS TRANSACTION " << transaction_queue.top().id << ".\n";
                    }
                    //remove order from queue
                    transaction_queue.pop();
                    continue;
                }
            }
            

            if (users[transaction_queue.top().sender].balance < (sender_fee + transaction_queue.top().amount))
            {
                //sender has insufficient funds

                if (verbose)
                {
                    cout << "INSUFFICIENT FUNDS TO PROCESS TRANSACTION " << transaction_queue.top().id << ".\n";
                }
                transaction_queue.pop();
                continue;
            }
            

            //At this point all checks are cleared and it is order execution time
           
            
            transaction_vec.push_back(transaction_queue.top());
            users[transaction_queue.top().sender].outgoing.push_back(transaction_queue.top());
            users[transaction_queue.top().recipient].incoming.push_back(transaction_queue.top());
            
            users[transaction_queue.top().sender].balance = 
                        users[transaction_queue.top().sender].balance - (sender_fee + transaction_queue.top().amount);

            if (transaction_queue.top().o_or_s == 's')
            {
                users[transaction_queue.top().recipient].balance += (transaction_queue.top().amount - recipient_fee);
                if (verbose)
                {
                    cout << "TRANSACTION " << transaction_queue.top().id << " EXECUTED AT " << transaction_queue.top().execution_date 
                    << ": $" << transaction_queue.top().amount << " FROM " << transaction_queue.top().sender
                    << " TO " << transaction_queue.top().recipient << ".\n";
                }
                
                transaction_queue.pop();
                continue;
            }
            
            if (verbose)
            {
                cout << "TRANSACTION " << transaction_queue.top().id << " EXECUTED AT " << transaction_queue.top().execution_date 
                    << ": $" << transaction_queue.top().amount << " FROM " << transaction_queue.top().sender
                    << " TO " << transaction_queue.top().recipient << ".\n";
            }
            
            users[transaction_queue.top().recipient].balance += (transaction_queue.top().amount);
            transaction_queue.pop();
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////                                         ////////////////////////////////////////////////////////
////////////////////////////     E  N  D     C  O  M  M  A  N  D     /////////////////////////////////////////////////////////
///////////////////////////                                         ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////                                /////////////////////////////////////////////////////////////
///////////////////////////////     BEGIN - Q  U  E  R  I  E  S   /////////////////////////////////////////////////////////////////
    ////////////////////////////////                              ///////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // This function runs the queries: list, revenue, history, and summary
    void run_queries(){

        uint64_t latest_stamp = 999999999999;
        process(latest_stamp);
        
        char val;
        string holder;
        uint64_t x_time = 0;
        uint64_t y_time = 0;
        
        while (cin >> val)
        {
            if (val == 'l')
            {
                cin >> holder;
                x_time = get_time_as_int(holder);
                cin >> holder;
                y_time = get_time_as_int(holder);
                list(x_time, y_time);
            }
            else if (val == 'r')
            {
                cin >> holder;
                x_time = get_time_as_int(holder);
                cin >> holder;
                y_time = get_time_as_int(holder);
                revenue(x_time, y_time);
            }
            else if (val == 'h')
            {
                string user_ID;
                cin >> user_ID;
                history(user_ID);
            } 
            else // val == 's'
            {
                string timeLOL;
                cin >> timeLOL;
                summary(timeLOL);
            }
        }
        
    }



    void list(uint64_t &x, uint64_t &y){

        uint32_t total_transactions = 0;
        string d = " DOLLARS";

         if (x == y)
         {
            cout << "LIST TRANSACTIONS REQUIRES A NON-EMPTY TIME INTERVAL.\n";
            return;
         }
         

        for (size_t i = 0; i < transaction_vec.size(); i++)
        {
            if ((transaction_vec[i].execution_date >= x) && (transaction_vec[i].execution_date < y))
            {
                total_transactions++;
                if (transaction_vec[i].amount == 1)
                {
                    d = " DOLLAR";
                    cout << transaction_vec[i].id << ": " << transaction_vec[i].sender << " SENT " << transaction_vec[i].amount 
                        << d << " TO " << transaction_vec[i].recipient << " AT " << transaction_vec[i].execution_date << ".\n";
                    d = " DOLLARS";
                }else
                {
                    cout << transaction_vec[i].id << ": " << transaction_vec[i].sender << " SENT " << transaction_vec[i].amount 
                        << d << " TO " << transaction_vec[i].recipient << " AT " << transaction_vec[i].execution_date << ".\n";
                }
                
            
            }


            if (transaction_vec[i].execution_date >= y)
            {
                break;
            }
        }
  

        if (total_transactions == 1)
        {
            cout << "THERE WAS 1 TRANSACTION THAT WAS PLACED BETWEEN TIME " << x << " TO " << y << ".\n";
            return;
        }
        
        cout << "THERE WERE " << total_transactions << " TRANSACTIONS THAT WERE PLACED BETWEEN TIME " << x << " TO " << y << ".\n";
    }// end list()



    void history(string user){
        auto it = users.find(user);
        if (it == users.end())
        {
            cout << "USER " << user << " DOES NOT EXIST.\n";
            return;
        }
        
        cout << "CUSTOMER " << user << " ACCOUNT SUMMARY:\n";
        cout << "BALANCE: $" << users[user].balance << "\n";
        cout << "TOTAL # OF TRANSACTIONS: " << (users[user].incoming.size() + users[user].outgoing.size()) << "\n";
        cout << "INCOMING " << users[user].incoming.size() << ":\n";

        
        if (users[user].incoming.size() > 10)
        {
            // 3: mmdarden sent 420 dollars to paoletti at 30004.
            for (size_t i = (users[user].incoming.size() - 10); i < users[user].incoming.size(); i++)
            {
                if (users[user].incoming[i].amount != 1)
                {
                    cout << users[user].incoming[i].id << ": " << users[user].incoming[i].sender << " SENT "
                    << users[user].incoming[i].amount << " DOLLARS TO " << users[user].incoming[i].recipient 
                    << " AT " << users[user].incoming[i].execution_date << ".\n";
                } else
                {
                    cout << users[user].incoming[i].id << ": " << users[user].incoming[i].sender << " SENT "
                    << users[user].incoming[i].amount << " DOLLAR TO " << users[user].incoming[i].recipient 
                    << " AT " << users[user].incoming[i].execution_date << ".\n";
                }
            }
            
        } else
        {
            for (size_t i = 0; i < users[user].incoming.size(); i++)
            {
                if (users[user].incoming[i].amount != 1)
                {
                    cout << users[user].incoming[i].id << ": " << users[user].incoming[i].sender << " SENT "
                    << users[user].incoming[i].amount << " DOLLARS TO " << users[user].incoming[i].recipient 
                    << " AT " << users[user].incoming[i].execution_date << ".\n";
                } else
                {
                    cout << users[user].incoming[i].id << ": " << users[user].incoming[i].sender << " SENT "
                    << users[user].incoming[i].amount << " DOLLAR TO " << users[user].incoming[i].recipient 
                    << " AT " << users[user].incoming[i].execution_date << ".\n";
                }
            }
        }
        
        cout << "OUTGOING " << users[user].outgoing.size() << ":\n";

        if (users[user].outgoing.size() > 10)
        {
            // 3: mmdarden sent 420 dollars to paoletti at 30004.
            for (size_t i = (users[user].outgoing.size() - 10); i < users[user].outgoing.size(); i++)
            {
                if (users[user].outgoing[i].amount != 1)
                {
                    cout << users[user].outgoing[i].id << ": " << users[user].outgoing[i].sender << " SENT "
                    << users[user].outgoing[i].amount << " DOLLARS TO " << users[user].outgoing[i].recipient 
                    << " AT " << users[user].outgoing[i].execution_date << ".\n";
                } else
                {
                    cout << users[user].outgoing[i].id << ": " << users[user].outgoing[i].sender << " SENT "
                    << users[user].outgoing[i].amount << " DOLLAR TO " << users[user].outgoing[i].recipient 
                    << " AT " << users[user].outgoing[i].execution_date << ".\n";
                }
            }
            
        } else//
        {
            for (size_t i = 0; i < users[user].outgoing.size(); i++)
            {
                if (users[user].outgoing[i].amount != 1)
                {
                    cout << users[user].outgoing[i].id << ": " << users[user].outgoing[i].sender << " SENT "
                    << users[user].outgoing[i].amount << " DOLLARS TO " << users[user].outgoing[i].recipient 
                    << " AT " << users[user].outgoing[i].execution_date << ".\n";
                } else
                {
                    cout << users[user].outgoing[i].id << ": " << users[user].outgoing[i].sender << " SENT "
                    << users[user].outgoing[i].amount << " DOLLAR TO " << users[user].outgoing[i].recipient 
                    << " AT " << users[user].outgoing[i].execution_date << ".\n";
                }
            }
        }
    } // END history()

    void summary(string time){

        uint64_t day;
        uint64_t next_day;
        int next_day_fake;
        string val;
        string next_d;

        val = time[6];
        val += time[7];
        next_day_fake = stoi(val);
        next_day_fake++;
        next_d = to_string(next_day_fake);
        if (next_d.size() == 1)
        {
            next_d.insert(1,6,'0');
        }else
        {
            next_d.insert(2,6,'0');
        }
        val.insert(2, 6, '0');

        day = stoll(val);
        next_day = stoll(next_d);

        string again1;
        string again2;
        string again3;
        string again4;
        again1 = time[0];
        again2 = time[1];
        again3 = time[3];
        again4 = time[4];
        uint64_t one = stoll(again1);
        one *= 100000000000;
        uint64_t two = stoll(again2);
        two *= 10000000000;
        uint64_t three = stoll(again3);
        three *= 1000000000;
        uint64_t four = stoll(again4);
        four *= 100000000;
        day = day + one + two + three + four;
        next_day = next_day + one + two + three + four;


        cout << "SUMMARY OF [" << day << ", " << next_day << "):\n";
        int total_trans = 0;
        uint32_t total_rev = 0;

        for (size_t i = 0; i < transaction_vec.size(); i++)
        {
            if ((transaction_vec[i].execution_date >= day) && (transaction_vec[i].execution_date < next_day))
            {
                total_trans++;
                total_rev += transaction_vec[i].fee;
                cout << transaction_vec[i].id << ": " << transaction_vec[i].sender << " SENT " 
                        << transaction_vec[i].amount; 
                if (transaction_vec[i].amount == 1)
                {
                    cout << " DOLLAR TO " << transaction_vec[i].recipient << " AT " 
                        << transaction_vec[i].execution_date << ".\n";
                }else
                {
                    cout << " DOLLARS TO " << transaction_vec[i].recipient << " AT " 
                        << transaction_vec[i].execution_date << ".\n";
                }
            }

            if (transaction_vec[i].execution_date >= next_day)
            {
                break;
            }
        }

        
        if (total_trans == 1)
        {
            cout << "THERE WAS A TOTAL OF " << total_trans << " TRANSACTION, 281BANK HAS COLLECTED " << total_rev
                << " DOLLARS IN FEES.\n";
        }
        else
        {
            cout << "THERE WERE A TOTAL OF " << total_trans << " TRANSACTIONS, 281BANK HAS COLLECTED " << total_rev
                    << " DOLLARS IN FEES.\n";
        }
        
    }



    void revenue(uint64_t &x, uint64_t &y){

        if (x == y)
        {
            cout << "BANK REVENUE REQUIRES A NON-EMPTY TIME INTERVAL.\n";
            return;
        }

        uint32_t total_fees_collected = 0;

        for (size_t i = 0; i < transaction_vec.size(); i++)
        {
            if ((transaction_vec[i].execution_date >= x) && (transaction_vec[i].execution_date < y))
            {
                total_fees_collected += transaction_vec[i].fee;
            }

            if (transaction_vec[i].execution_date >= y)
            {
                break;
            }
        }

        string string_time = get_uint_as_string((y - x));
        int val;
        string hold;
        int int_holder;
        
        cout << "281BANK HAS COLLECTED " << total_fees_collected << " DOLLARS IN FEES OVER";
        hold = string_time[0];
        int_holder = stoi(hold);
        val = 10 * int_holder;
        hold = string_time[1];
        int_holder = stoi(hold);
        val += int_holder;
        if (val != 0)
        {
            cout << " " << val;
            if (val == 1)
            {
                cout << " YEAR";
            }
            else
            {
                cout << " YEARS";
            }
        }

        hold = string_time[3];
        int_holder = stoi(hold);
        val = 10 * int_holder;
        hold = string_time[4];
        int_holder = stoi(hold);
        val += int_holder;
        if (val != 0)
        {
            cout << " " << val;
            if (val == 1)
            {
                cout << " MONTH";
            }
            else
            {
                cout << " MONTHS";
            }
        }
        
        hold = string_time[6];
        int_holder = stoi(hold);
        val = 10 * int_holder;
        hold = string_time[7];
        int_holder = stoi(hold);
        val += int_holder;
        if (val != 0)
        {
            cout << " " << val;
            if (val == 1)
            {
                cout << " DAY";
            }
            else
            {
                cout << " DAYS";
            }
        }
      
        hold = string_time[9];
        int_holder = stoi(hold);
        val = 10 * int_holder;
        hold = string_time[10];
        int_holder = stoi(hold);
        val += int_holder;
        if (val != 0)
        {
            cout << " " << val;
            if (val == 1)
            {
                cout << " HOUR";
            }
            else
            {
                cout << " HOURS";
            }
        }

        hold = string_time[12];
        int_holder = stoi(hold);
        val = 10 * int_holder;
        hold = string_time[13];
        int_holder = stoi(hold);
        val += int_holder;
        if (val != 0)
        {
            cout << " " << val;
            if (val == 1)
            {
                cout << " MINUTE";
            }
            else
            {
                cout << " MINUTES";
            }
        }

        hold = string_time[15];
        int_holder = stoi(hold);
        val = 10 * int_holder;
        hold = string_time[16];
        int_holder = stoi(hold);
        val += int_holder;
        if (val != 0)
        {
            cout << " " << val;
            if (val == 1)
            {
                cout << " SECOND";
            }
            else
            {
                cout << " SECONDS";
            }
        }
        cout << ".\n";
    } // end revenue()


public:

    //constructor:
    Bank(string file, bool v, uint64_t time) : reg_file(file), verbose(v), current_timestamp(time) {}


    void read_reg_file(){
        std::ifstream file(reg_file); // Open the file

        //User newUser;
        uint64_t time = 0;
        string id = " ";
        string pin = " ";
        string real_b = " ";

        string val;
        int i = 0;
        while (getline(file, val,'|'))
        {
            //
            if (i == 0)
            {
                //val = time
                time = get_time_as_int(val);
            } else if (i == 1)
            {
                //val = user
                id = val;
            }else if (i == 2)
            {
                //val = pin
                pin = val;
                getline(file, real_b);

                User newUser;
                newUser.balance = stoi(real_b);
                newUser.id = id;
                newUser.pin = pin;
                newUser.time = time;
        
                //real_b = balance
                users[newUser.id] = newUser;
            }
            i++;
            
            if (i > 2)
            {
                i = 0;
            }
            
        }
    }// End read reg file




    void read_commands(){

        string user_id;
        string recipient;
        string ipAddress;
        string pin;
        string transaction_stamp;
        uint64_t transaction_stamp_n;
        uint32_t amount_n;
        string execution;
        uint64_t execution_n;
        char trans_fee;
        int transaction_id = 0;

        string val;
        while (cin >> val)
        {
            char commandIdentifier = val[0];

            if (commandIdentifier == '#') //comment in test file - ignore
            {
                getline(cin, val);
            }
            else if (commandIdentifier == '$') //queries in test file - run queries()
            {
                run_queries();
            } 
            else if (commandIdentifier == 'l') //login in test file - login()
            {
                cin >> user_id;
                cin >> pin;
                cin >> ipAddress;
                login_func(ipAddress, user_id, pin);

            } else if (commandIdentifier == 'o') //logout in test file - logout()
            {
                cin >> user_id;
                cin >> ipAddress;
                logout_func(ipAddress, user_id);

            } else if (commandIdentifier == 'b') //balance requested in test file - balance()
            {
                cin >> user_id;
                cin >> ipAddress;
                balance_func(user_id, ipAddress);

            } 
            
            else // == 'p'  , place command in test file - place() 
            {
                cin >> transaction_stamp;
                current_time = transaction_stamp;
                transaction_stamp_n = get_time_as_int(transaction_stamp);
                
                cin >> ipAddress;
                cin >> user_id;
                cin >> recipient;
                cin >> amount_n;
                cin >> execution;
                execution_n = get_time_as_int(execution);
                cin >> trans_fee;

                //first check for errors in place command:

                if (transaction_stamp_n < current_timestamp)
                {
                    cout << "INVALID DECREASING TIMESTAMP IN 'PLACE' COMMAND.";
                    exit(1);
                }
                else
                {
                    current_timestamp = transaction_stamp_n;
                }
                
                if (execution_n < transaction_stamp_n)
                {
                    // execution placed for a time previous to current time, error print
                    // cout << "execution date is in the past, not possible\n";
                    cout << "YOU CANNOT HAVE AN EXECUTION DATE BEFORE THE CURRENT TIMESTAMP.";
                    exit(1);
                }

                //check that sender is different from recipient
                if (user_id == recipient)
                {
                    if (verbose)
                    {
                        cout << "SELF TRANSACTIONS ARE NOT ALLOWED.\n";
                    }

                    continue;
                }

                //now check account start dates, and that exec date is within three days of now.
                uint64_t difference = execution_n - transaction_stamp_n;
                if (difference > 3000000)
                {
                    // execution more than three days, error print
                    if (verbose)
                    {
                        cout << "SELECT A TIME UP TO THREE DAYS IN THE FUTURE.\n";
                    }
                    
                    continue;
                }
                
                // sender exists?
                auto it = users.find(user_id);

                if (it == users.end())//sender does not exist
                {
                    
                    if (verbose)
                    {
                        cout << "SENDER " << user_id << " DOES NOT EXIST.\n";
                    }
                    
                    continue;
                }

                //recipient exists?
                auto it2 = users.find(recipient);

                if (it2 == users.end()) //recipient does not exist
                {
                    if (verbose)
                    {
                        cout << "RECIPIENT " << recipient << " DOES NOT EXIST.\n";
                    }
                    
                    continue;
                }

                //checking both party's registration date
                if ((users[user_id].time > execution_n)||(users[recipient].time > execution_n))
                {
                    // either r or s or both do not have accounts at execution time
                    if (verbose)
                    {
                        cout << "AT THE TIME OF EXECUTION, SENDER AND/OR RECIPIENT HAVE NOT REGISTERED.\n";
                    }
                    
                    continue;
                }

                //checking for active sender session
                if (users[user_id].IPs.empty()) //sender is not logged in
                {
                    if (verbose)
                    {
                        cout << "SENDER " << user_id << " IS NOT LOGGED IN.\n";
                    }
                    
                    continue;
                }
                
                
                //checking IP
                auto it3 = users[user_id].IPs.find(ipAddress);

                if (it3 == users[user_id].IPs.end()) //ip does not exist in sender IPset
                {
                    if (verbose)
                    {
                        cout << "FRAUDULENT TRANSACTION DETECTED, ABORTING REQUEST.\n";
                    }
                    //ip doesnt exists in sender IPset
                    continue;
                }

                // IF HERE, THEN ALL ERROR CHECKS PASSED
                // Now, push new transaction to PQ

                // setting the transaction fee
                uint32_t fee = amount_n / 100;

                if (fee > 450) //fee too high
                {
                    fee = 450;
                }
                else if (fee < 10)
                {
                    fee = 10;
                }
                
                if ((execution_n - users[user_id].time) > 50000000000)
                {
                    fee = (fee*3) / 4;
                }

                Transaction holder;
                holder.fee = fee;
                holder.amount = amount_n;
                holder.execution_date = execution_n;
                holder.id = transaction_id;
                holder.o_or_s = trans_fee;
                holder.recipient = recipient;
                holder.sender = user_id;

                transaction_id++;

                process(transaction_stamp_n);

                if (verbose)
                {
                    cout << "TRANSACTION " << transaction_id - 1 << " PLACED AT " << transaction_stamp_n
                    << ": $" << amount_n << " FROM " << user_id << " TO " << recipient << " AT " << execution_n << ".\n";
                }

                transaction_queue.push(holder);
            }
        }
    }


}; // End Bank class



int main(int argc, char *argv[]){

    Options options;
    getOptions(argc, argv, options);


    Bank bank(options.reg_file, options.verbose, 0);
    bank.read_reg_file();
    bank.read_commands();

    
    


}
