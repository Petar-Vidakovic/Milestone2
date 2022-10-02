# 2803ICT - Milestone 2

## Description

The task is to establish a client server communication over a connected network of sockets. The task is to host a game over a network. The game is unimportant and is simply sending text over the network.

### Run the makefile, Jimmy!:

    make all 

which will compile both the server and client programs.

### If the makefile does not work compile both programs with the following commands

    gcc -o serv server.c
    gcc -o cli client.c

Then run the server and client with arguments

    ./server (port number) (numbers) (players)

Then run the clients with,

    ./client (numbers) (PetarServer) (port number)

The program will prompt you if you get it wrong.

Launch as many clients as (players). The server will wait until all players have joined the server the game will start.
During the play cycle of the game players can only enter digits 1 to 9 or the disconnect word “quit”. In the order the
players join the server will send the current score and a GO message which signals the corresponding connection to make
their turn. If a client is not responsive for 30 second their connection will be closed and exit the game. The game will
continue until there is only 1 player. When there is a winner messages will be sent to all connected clients that they
either won or lost.

## Authors

Petar Vidakovic - s5240487 
