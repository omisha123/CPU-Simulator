#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

struct Task {
    string id;
    int rem;
    vector<string> mem;
    size_t idx;

    Task(string i, int b, vector<string> m)
        : id(i), rem(b), mem(m), idx(0) {}
};

class Cache {
    string name;
    size_t cap;
    vector<string> store;
public:
    Cache(string n, size_t c) : name(n), cap(c) {}
    bool hit(const string& blk) { return find(store.begin(), store.end(), blk) != store.end(); }
    
    void insert(const string& blk) {
        if (hit(blk)) return;
        if (store.size() >= cap) {
            store.erase(store.begin()); 
        }
        store.push_back(blk);
    }
    
    void show() const {
        cout << name << ": [";
        for (size_t i = 0; i < store.size(); ++i) {
            cout << store[i];
            if (i < store.size() - 1) cout << ", ";
        }
        cout << "]";
    }
};

class MemSys {
    Cache l1, l2, l3;
    int ramcount, cycles;
public:
    MemSys() : l1("L1", 32), l2("L2", 128), l3("L3", 512), ramcount(0), cycles(0) {}
    int ram() const { return ramcount; }
    int totalCycles() const { return cycles; }

    void access(const string& blk) {
        cout << "Access " << blk << "\n";
        l1.show();
        if (l1.hit(blk)) { cout << " -> HIT (4 cyc)\n"; cycles += 4; return; }
        cout << " >> MISS\n";
        
        l2.show();
        if (l2.hit(blk)) {
            cout << " >> HIT (12 cyc) promote " << blk << " -> L1\n";
            cycles += 12;
            l1.insert(blk);
            l1.show(); cout << "\n";
            return;
        }
        cout << " >> MISS\n";
        
        l3.show();
        if (l3.hit(blk)) {
            cout << " >> HIT (40 cyc) promote " << blk << " -> L1\n"; // Fixed text
            cycles += 40;
            l1.insert(blk); 
            l1.show(); cout << "\n";
            return;
        }
        cout << " >> MISS\n";
        
        cout << "Fetch from RAM (200 cyc)\n";
        ramcount++; cycles += 200;
        l3.insert(blk);
        l2.insert(blk);
        l1.insert(blk);
        l3.show(); cout << "\n";
        l2.show(); cout << "\n";
        l1.show(); cout << "\n";
    }
};

vector<Task> readTasks(const string& filename) {
    ifstream fin(filename);
    vector<Task> tasks;
    string line;

    while (getline(fin, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == string::npos) 
            continue;

        stringstream ss(line);
        string token;
        ss >> token;
        
        if (token == "TASK") {
            string id;
            string burst_label, mem_label;
            int burst;
            
            ss >> id >> burst_label >> burst >> mem_label;
            
            vector<string> mems;
            string blk;
            while (ss >> blk) {
                mems.push_back(blk);
            }
            
            tasks.push_back(Task(id, burst, mems));
        }
    }
    return tasks;
}

int main() {
    vector<Task> all = readTasks("input.txt"); 
    
    if (all.empty()) {
        cerr << "Error: No tasks loaded from input.txt!\n";
        return 1;
    }

    queue<Task> q;
    MemSys mem1;
    int cyc = 1, done = 0;
    const int Q = 3;
    size_t nxt = 0; 

    while (done < all.size() || !q.empty()) {
        
        if (nxt < all.size()) {
            q.push(all[nxt]);
            cout << "Task " << all[nxt].id << " arrived at cyc " << cyc << "\n";
            nxt++;
        }

        if (!q.empty()) {
            Task cur = q.front(); q.pop();
            int slice = min(Q, cur.rem);
            
            for (int i = 0; i < slice; i++) {
                cout << "Cycle " << cyc << " - Run " << cur.id;
                if (!cur.mem.empty()) {
                    string r = cur.mem[cur.idx];
                    cout << " Req: " << r << "\n";
                    
                    int start_mem_cycles = mem1.totalCycles();
                    mem1.access(r);
                    int elapsed_mem_cycles = mem1.totalCycles() - start_mem_cycles;
                    
                    cur.idx = (cur.idx + 1) % cur.mem.size();
                    cyc += max(1, elapsed_mem_cycles);
                } else {
                    cout << " Req: None\n";
                    cyc++;
                }
                cur.rem--;
                
                if (nxt < all.size()) {
                    q.push(all[nxt]);
                    cout << "Task " << all[nxt].id << " arrived at cyc " << cyc << "\n";
                    nxt++;
                }
            }
            
            if (cur.rem > 0) {
                q.push(cur);
            } else { 
                done++; 
                cout << "Task " << cur.id << " done.\n"; 
            }
        }
    }

    cout << "\n=== Final Results ===\n";
    cout << "Total Cycles: " << cyc - 1 << "\n";
    cout << "Tasks Completed: " << done << "\n";
    cout << "Scheduler: Round Robin (Q=" << Q << ")\n";
    cout << "RAM Accesses: " << mem1.ram() << "\n";
    cout << "Memory Cycles: " << mem1.totalCycles() << "\n";

    return 0;
}
