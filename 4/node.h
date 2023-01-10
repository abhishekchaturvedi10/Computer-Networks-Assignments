#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <bits/stdc++.h>

using namespace std;

const int INF = 1e9 ;

/*
  Each row in the table will have these fields
  dstip:	Destination IP address
  nexthop: 	Next hop on the path to reach dstip
  ip_interface: nexthop is reachable via this interface (a node can have multiple interfaces)
  cost: 	cost of reaching dstip (number of hops)
*/
class RoutingEntry{
 public:
  string dstip, nexthop;
  string ip_interface;
  int cost;
};

/*
 * Class for specifying the sort order of Routing Table Entries
 * while printing the routing tables
 * 
*/
class Comparator{
 public:
  bool operator()(const RoutingEntry &R1,const RoutingEntry &R2){
    if (R1.cost == R2.cost) {
      return R1.dstip.compare(R2.dstip)<0;
    }
    else if(R1.cost > R2.cost) {
      return false;
    }
    else {
      return true;
    }
  }
} ;

/*
  This is the routing table
*/
struct routingtbl {
  vector<RoutingEntry> tbl;
};

/*
  Message format to be sent by a sender
  from: 		Sender's ip
  mytbl: 		Senders routing table
  recvip:		Receiver's ip
*/
class Node ;

class RouteMsg {
 public:
  Node* from ;			         // I am sending this message, so it must be me i.e. if A is sending mesg to B then it is A
  struct routingtbl mytbl ;	    // This is routing table of A
};

/*
  Emulation of network interface. Since we do not have a wire class, 
  we are showing the connection by the pair of IP's
  
  ip: 		Own ip
  connectedTo: 	An address to which above mentioned ip is connected via ethernet.
*/
class NetInterface {
 private:
  string ip;
  string connectedTo; 	//this node is connected to this ip
  int cost ; 
  
 public:
  string getip() {
    return this->ip;
  }
  string getConnectedIp() {
    return this->connectedTo;
  }

  int getcost(){
	return this->cost ; 
  }
  void setip(string ip) {
    this->ip = ip;
  }
  void setConnectedip(string ip) {
    this->connectedTo = ip;
  }

  void setcost(int cost){
	this->cost = cost ;
  }
  
};

/*
  Struct of each node
  name: 	It is just a label for a node
  interfaces: 	List of network interfaces a node have
  Node* is part of each interface, it easily allows to send message to another node
  mytbl: 		Node's routing table
*/
class Node {
 private:
  
  string name;
  vector<pair<NetInterface, Node*> > interfaces;
 
 protected:
  
  struct routingtbl mytbl;
  virtual void recvMsg(RouteMsg* msg) {
    cout<<"Base"<<endl;
  }

 public:
  void setName(string name){
    this->name = name;
  }

    bool isMyInterface(string eth) {
		
		for (int i = 0; i < interfaces.size(); ++i) {
			if(interfaces[i].first.getip() == eth){
				return true;
			}
		}

		return false;
	}
  
  void addInterface(string ip, string connip, int cost, Node *nextHop) {
    NetInterface eth;
    eth.setip(ip);
    eth.setConnectedip(connip);
	eth.setcost(cost) ;
    interfaces.push_back({eth, nextHop});
  }

  
  
  void addTblEntry(string dstip, string nexthop, string myip, int cost) {
    RoutingEntry entry;
    entry.dstip = dstip;
    entry.nexthop = nexthop;
    entry.ip_interface = myip;
    entry.cost = cost;
    mytbl.tbl.push_back(entry);
  }

  void updateTblEntry(string _myip, string _dstip, string _nexthop, int _cost) {
    
	// to update the dstip hop count in the routing table (if dstip already exists)
    // new hop count will be equal to the cost 

    for (int i=0; i<mytbl.tbl.size(); i++){
      
	  RoutingEntry entry = mytbl.tbl[i];

      if (entry.dstip == _dstip){
		mytbl.tbl[i].ip_interface = _myip;
		mytbl.tbl[i].nexthop = _nexthop ;
		mytbl.tbl[i].cost = _cost ;
		return ; 
	  }
    }

	this->addTblEntry(_dstip, _nexthop, _myip, _cost) ;

  }

	void initialise_RT(){

		for(int i=0 ; i<interfaces.size() ; i++){

			string myip = interfaces[i].first.getip() ;
			string dstip = interfaces[i].first.getConnectedIp() ;
			int cost = interfaces[i].first.getcost() ;

			this->addTblEntry(dstip, dstip, myip, cost) ; 
			this->addTblEntry(myip, myip, myip, 0) ; 
		}
	}

	vector<string> get_interfaces(){

		vector<string> ips ; 

		for(int i=0 ; i<interfaces.size() ; i++){
			ips.push_back(interfaces[i].first.getip()) ; 
		} 

		return ips ; 
	}
  
  string getName() {
    return this->name;
  }
  
  struct routingtbl getTable() {
    return mytbl;
  }
  
  void printTable() {
    
	Comparator myobject;
    sort(mytbl.tbl.begin(),mytbl.tbl.end(),myobject);

    cout<<this->getName()<<":"<<endl;
	// cout<<"destIP"<<" "<<"Nexthop"<<" "<<"myinterfaceIP"<<" " << "Cost" << endl ; 

    for (int i = 0; i < mytbl.tbl.size(); ++i) {
      cout<<mytbl.tbl[i].dstip<<" | "<<mytbl.tbl[i].nexthop<<" | "<<mytbl.tbl[i].ip_interface<<" | "<<mytbl.tbl[i].cost <<endl;
    }

  }
  
	RouteMsg sendMsg(){

		RouteMsg sendMsg ; 
		sendMsg.from = this ; 
		sendMsg.mytbl = (this -> mytbl) ; 
		return sendMsg  ; 

    // struct routingtbl ntbl;
    // for (int i = 0; i < mytbl.tbl.size(); ++i) {
    //   ntbl.tbl.push_back(mytbl.tbl[i]);
    // }
    
    // for (int i = 0; i < interfaces.size(); ++i) {
    //   RouteMsg msg;
    //   msg.from = interfaces[i].first.getip();
    //   //printf("i=%d, msg-from-interface=%s\n",i, msg.from.c_str());
    //   msg.mytbl = &ntbl;
    //   msg.recvip = interfaces[i].first.getConnectedIp();		
    //   interfaces[i].second->recvMsg(&msg);
    // }
  }
  
};

class container {

  public :
	int cost ; 
	Node* currNode ; 
	string srcinterface ;
	string destinterface ;
	string nexthop ;

	container(){}

	void set_interfaces(int cost, Node* node, string s, string d, string n){
		this -> cost = cost ;
		this -> currNode = node ; 
		this -> srcinterface = s ; 
		this -> destinterface = d ;
		this -> nexthop = n ;
	}
};

struct comp {
	bool operator()(container A, container B){
		return A.cost > B.cost ; 
	}
};

class RoutingNode: public Node {
 
 public:
  
	void recvMsg(vector<RouteMsg> &GLOBAL_RTs){

		map<string, int> dist ;    // Name of the node and dist to it 
		map<string, bool> done ;

		for(int i=0 ; i<GLOBAL_RTs.size() ; i++){
			dist[GLOBAL_RTs[i].from->getName()] = INF ;
			done[GLOBAL_RTs[i].from->getName()] = false ; 
		}

		container src ; 
		src.set_interfaces(0, this, "", "", "" ) ; 

		dist[this->getName()] = 0 ; 

		// Dist, currnode, srcusedInterface, destusedInterface, nexthoptoreach ;

		priority_queue<container,vector<container>, comp> pq ;   
		pq.push(src) ; 

		while(!pq.empty()){

			container curr = pq.top() ; 
			pq.pop() ; 

			Node* currnode = curr.currNode ; 

			// cout << currnode -> getName() << " " << curr.cost << " " << curr.srcinterface << " " << curr.destinterface << endl ; 
			
			if(done[currnode -> getName()]){ continue ; }   // To reduce T.C ;
            done[currnode -> getName()] = true ;

			if(curr.srcinterface != ""){

				vector<string> curr_interfaces = currnode -> get_interfaces() ; 
				 
				for(int i=0 ; i<curr_interfaces.size() ; i++){
					this->updateTblEntry(curr.srcinterface, curr_interfaces[i], curr.nexthop, curr.cost) ; 
				}
			}
			
			int currcost = curr.cost ; 
			RouteMsg currNodeMsg ; 

			for(int i=0 ; i<GLOBAL_RTs.size() ; i++){

				if(GLOBAL_RTs[i].from->getName() == currnode->getName()){
					currNodeMsg = GLOBAL_RTs[i] ;
					break ;  
				}
			}

			routingtbl currtbl = currNodeMsg.mytbl ; 

			for(int i=0 ; i<currtbl.tbl.size() ; i++){

				string currInterface = currtbl.tbl[i].ip_interface ; 
				string nextInterface = currtbl.tbl[i].nexthop ;
				int cost = currtbl.tbl[i].cost ;

				Node* nextnode ; 

				for(int i=0 ; i<GLOBAL_RTs.size() ; i++){
					if(GLOBAL_RTs[i].from->isMyInterface(nextInterface)){
						nextnode = GLOBAL_RTs[i].from ; 
						break ; 
					}
				}

				if(dist[nextnode->getName()] > currcost + cost){

					dist[nextnode->getName()] = currcost + cost ;

					if(curr.srcinterface == ""){
						container x ; 
						x.set_interfaces(currcost + cost, nextnode, currInterface, nextInterface, nextInterface) ;  
						pq.push(x) ;
					}

					else{
						container x ; 
						x.set_interfaces(currcost + cost, nextnode, curr.srcinterface, nextInterface, curr.nexthop) ;
						pq.push(x) ;
					}
				}
			}
		}

		// cout << endl ; 
	}
};
