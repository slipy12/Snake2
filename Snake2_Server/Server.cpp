#include "Server.h"



Server::Server(unsigned port)
{
	this->port = port;
}


Server::~Server()
{
}

void Server::run()
{
	next_update = 0;
	next_move = 0;
	if (socket.bind(port) != sf::Socket::Done) {
		cout << "Coudn't start server: bind failed" << endl;
		return;
	}
	cout << "Server is listening to port " << port << endl;
	cout << "Protocol message size: " << sizeof(Message) << endl;
	socket.setBlocking(false);
	char in[sizeof(Message)];
	memset(in, '\0', sizeof(Message));
	std::size_t received;
	sf::IpAddress sender;
	unsigned short senderPort;
	sf::Clock clock;
	int previous = clock.getElapsedTime().asMilliseconds();
	game_data = vector<Snake>(0);
	while (true) {
		sf::Socket::Status s = socket.receive(in, sizeof(in), received, sender, senderPort);
		if (received != 0) {
			Message got = Protocol::decode(in);
			if (got.t < Message::MAX) {
				cout << (string)sender.toString() << endl;
					cout<< to_string(senderPort) << endl;
				string UUID = (string)(sender.toString() + to_string(senderPort));
				if (got.t == Message::JOIN) {
					User newUser;
					newUser.ip = sender;
					newUser.port = senderPort;
					newUser.sID = game_data.size();
					users[UUID] = newUser;
					Snake tempSnake;
					tempSnake.parts = std::vector<Part>(0);
					int div = game_data.size() % 4;
					int x0 = (rand() % (WIDTH / 2 - MIN_PARTS)) + MIN_PARTS;
					int dx = -1;
					if (div > 1) {
						dx = 1;
						x0 += WIDTH / 2;
					}
					int y0 = rand() % (HEIGHT / 2);
					if (div % 2 == 1)
						y0 += HEIGHT / 2;
					for (int i = 0; i < MIN_PARTS; i++)
					{
						tempSnake.parts.push_back({ (char)(x0 + i * dx), char(y0) });
					}
					if (dx == 1) tempSnake.direction = RIGHT;
					else tempSnake.direction = LEFT;
					game_data.push_back(tempSnake);
					cout << "NEW USER Current users: " << users.size() << endl;
				}
				if (users.count(UUID)>0) {
					User *u = &users[UUID];
					Message tobroadcast=Protocol::make(Message::NONE);
					Message res = u->message(got,game_data, tobroadcast);
					if (tobroadcast.t!=Message::NONE) broadcast(UUID, tobroadcast);
					if (res.t != Message::NONE) socket.send(Protocol::encode(res), sizeof(Message) , u->ip, u->port);
				}
			}
		}
		int current = clock.getElapsedTime().asMilliseconds();
		if (current != previous) {
			int steps = current - previous;
			if (users.size() > 0) {
				for (int s = 0; s < steps; ++s) {
					update();
				}
				next_update += steps;
				if (next_update > UPDATEEVERY) {
					//broadcastAll(Protocol::update(next_move,game_data));
					next_update -= UPDATEEVERY;
				}
			}
			previous = current;
		}
	}
}

void Server::update()
{/*
	++next_move;
	int mt = (int)(game_data.size() * 2 + EXTRA);
	if (next_move >= mt) {
		next_move -= mt;
		for (unsigned i = 0; i < game_data.size(); ++i) {
			int dir = ((game_data[i].type&SHIP_RIGHT)>0) ? 1 : -1;
			int nx = game_data[i].x + 5 * dir;
			if (nx<BORDER || nx>WIDTH - BORDER) {
				game_data[i].type = (game_data[i].type&7) | (game_data[i].type ^ (SHIP_RIGHT | SHIP_LEFT));
				dir = ((game_data[i].type&SHIP_RIGHT)>0) ? 1 : -1;
				nx = game_data[i].x + 5 * dir;
			}
			game_data[i].x = nx;
		}
	}*/
}

void Server::broadcast(string UUID, const Message &m)
{
	const char *data = Protocol::encode(m);
	for (auto it = users.begin(); it != users.end(); ++it) {
		if (it->first != UUID) {
			socket.send(data, sizeof(Message), it->second.ip, it->second.port);
		}
	}
}

void Server::broadcastAll(const Message & m){
	const char *data = Protocol::encode(m);
	for (auto it = users.begin(); it != users.end(); ++it) {
		socket.send(data, sizeof(Message), it->second.ip, it->second.port);
	}
}