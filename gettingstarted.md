
# Getting Started
Read readme.md for background on the engine and how to build it, this document describes how to use it if you think it is right for you.

## Background Reading
- For an overview of what an HTN is read: https://www.exospecies.com/blog/htnoverview
- For an overview of the Prolog engine used inside the Inductor HTN Engine read: https://www.exospecies.com/blog/prolog
- For an overview of how to use the HTN Language for the Inductor HTN Engine read: [Not posted yet]
- For a lot of background on how this HTN Engine was used in production in a strategy game, start at the first blog entry on https://www.exospecies.com/blog and read through to the bitter end.

## Optimal Problems for HTNs

Hierarchical Task Networks are a proven model for solving many AI Planning problems and they've been around for a long time. I've found that they are a good solution if you need an engine that can create a plan in a complex problem space where doing an exhaustive search (or an approximation of it) to solve the problem simply isn't an option AND where you have an expert that knows the right answer (or a good enough answer) because they're going to have to code up the rules.  Your HTN will only do its job as well as the best person you have writing the rules.

One example where I think HTN's *shouldn't* be used: Two-person zero-sum games with perfect information (Chess, Checkers, etc.), I suspect that some variant of the [minimax algorithm](https://en.wikipedia.org/wiki/Minimax) is going to be your best bet.  This does an exhaustive search, or close enough for many purposes.

HTN's were a great solution for [Exospecies](www.exospecies.com) because it is a complex game with resource management and the high cost of calculating a turn makes running lots of scenarios (like minimax) impossible.  An approach that used rules written by an expert was the best I was going to do. That's what the Inductor HTN Engine was originally built for and where it was first used in production.  It uses Prolog as a primary part of its language and the [Inductor Prolog Engine](https://github.com/EricZinda/InductorProlog) as part of its runtime engine.




## Usage Overview
The InductorHtn engine adds HTN capabilities on top of what is basically a classic Prolog compiler (the [Inductor Prolog Compiler](https://github.com/EricZinda/InductorProlog)). So, understanding Prolog is key to using this engine.  In fact, you can use all the normal Prolog features implemented in Inductor Prolog as a part of your HTN application and "mix and match" HTN constructs alongside Prolog constructs. Background reading to get you up to speed on Prolog is in the section above.

There are three steps to using this engine in an application:
1. Convert the app state you need to process into Prolog Facts 
2. Write the HTN Axioms, Methods, and Operators you need and use the Facts
3. Convert the Operators that get generated into changes, moves, or whatever makes sense in your app

To make it easy to prototype or try out the engine, the build system builds an interactive mode application called `indhtn`. The next section describes how to use it.

## Using Interactive Mode
The easiest way to use interactive mode is to create a single file with a `.htn` extension and pass it on the command line. You can write down the facts that will be input to the engine, the HTN Axioms, Methods, and Operators that are your HTN logic and run it interactively. There is a tiny amount of help built into the app that should get you going.

## An Example
Let's take the [Exospecies](www.exospecies.com) game and boil it down to be simple enough for an example. We'll design an AI for a simple tile-based game akin to Chess that has a simple set of rules:

- The game "map" consists of tiles all connected in a square
- The game consists of units, a tile can only hold one unit
- If you move into a square already occupied by an opponent you take that piece off the board
- One unit is a King, if you take the opponent's King you win.
- The rest of the units are pawns that can move one square at a time
- You each start on opposite ends of the map with 3 pawns and a king

Imagine you have written that game meant to be played on a PC, iPhone, etc and you want to write a computer player.  

NOTE: As I said above, this is a perfect scenario for an algorithm called Minimax, and that algorithm will probably generate a *much* better AI Player.  However, I am using it as a proxy for a "complicated strategy game that can't be solved using Minimax" so bear with me. 

Let's go through the process.

## First, play the game
The game AI is implemented and fully playable, but the interface is a little painful since it is implemented in the interactive mode described above.  Later in this document all of the code will be described. Here's how to play:

Load the game from the command line (using the right paths and syntax for your OS):
~~~
./indhtn ../../Examples/Game.htn
~~~

You can see the current state of the board by using `b.`:
~~~
?- b.
tile(0,0) Pawn2-2   King2-1   Pawn2-3   tile(4,0)
tile(0,1) tile(1,1) Pawn2-1   tile(3,1) tile(4,1)
tile(0,2) tile(1,2) tile(2,2) tile(3,2) tile(4,2)
tile(0,3) tile(1,3) Pawn1-1   tile(3,3) tile(4,3)
tile(0,4) Pawn1-2   King1-1   Pawn1-3   tile(4,4)
>> ((), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), ())
~~~
Pawns and kings have a number after them which indicates which player they belong to (e.g. Pawn2-1 is Player 2's pawn #1).
Tiles that don't have a unit on them just have the address of the tile like `tile(0,0)`

To move a unit use the HTN Method called `tryMove(?Unit, ?Destination)` using the `apply()` command so the state is retained like this:
~~~
?- apply(tryMove(Pawn1-1, tile(2,2))).
>> (doMove(Pawn1-1,tile(2,3),tile(2,2)))

?- b.
tile(0,0) Pawn2-2   King2-1   Pawn2-3   tile(4,0)
tile(0,1) tile(1,1) Pawn2-1   tile(3,1) tile(4,1)
tile(0,2) tile(1,2) Pawn1-1   tile(3,2) tile(4,2)
tile(0,3) tile(1,3) tile(2,3) tile(3,3) tile(4,3)
tile(0,4) Pawn1-2   King1-1   Pawn1-3   tile(4,4)
>> ((), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), ())
~~~

To get the AI to take its turn, `apply()` the top-level method called `doAI(?Player)` and tell it which side you want it to play.  You can mix and match to try things out since the AI is stateless and just takes the best move given the state of the board:
~~~
?- apply(doAI(Player2)).
>> (captureUnit(Pawn1-1,tile(2,2)), doMove(Pawn2-1,tile(2,1),tile(2,2)))

?- b.
tile(0,0) Pawn2-2   King2-1   Pawn2-3   tile(4,0)
tile(0,1) tile(1,1) tile(2,1) tile(3,1) tile(4,1)
tile(0,2) tile(1,2) Pawn2-1   tile(3,2) tile(4,2)
tile(0,3) tile(1,3) tile(2,3) tile(3,3) tile(4,3)
tile(0,4) Pawn1-2   King1-1   Pawn1-3   tile(4,4)
>> ((), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), (), ())
~~~
When either side takes the King, nothing happens since it is just a sample.  Sorry!

## Cool things about the Example AI
Because this example is built on Prolog and HTNs, there are bunch of things you just get "for free"! Because of the tree exploring, query running nature or Prolog and HTNs, a bunch of things often fall out that are surprising.  

Some of these may not make sense until you read the code below, but I'm putting them up front to help propel you forward!

### Arbitrary Maps!
You should be able to leave gaps in tiles, make arbitrarily large surfaces, non-square surfaces, etc. The code above just works because it is designed around checking for tile(?X,?Y) terms, and Axioms like `filledSquare()` and others will not explore parts of the tree that don't have tiles that actually exist. (Note that the map display logic in the game will not be pretty, though!)

### Arbitrary numbers of units...and Kings!
Nothing in the code doesn't care how many of anything there are.  It will defend as many kings as you have, use as many units as you have, etc.  Go ahead and add more in Game.htn!

### Pathfinding!
If you build maps that have gaps in them, units will route around the gaps magically since we built the routines that move to explore all alternatives.  It won't be guaranteed to be the "shortest path", however.

### Alternative Solutions!
When you use `goal(doAI(Player1)).` in interactive mode, you'll often see many solutions, and we've just been picking the first.  Turns out that, in some applications, looking through the alternatives for secondary benefits in a "second pass" might make you choose the next best solutions instead.

### Easy to add new types of units!
Simply use a new name of unit when you declare it like: `unit(Warrior2-3, Warrior, Player2).`, no other changes needed! The AI will still properly attack it, defend against it, etc.  If it has behavior that is different enough, you'll of course want to write new rules about it.

## Building the AI
First step in using HTNs is to represent the game or app you are targeting in a form the HTN engine can understand: Prolog facts. 

For this example, representing the tiles is easy, let's just invent a term called `tile` that has an X and Y coordinate.  Each term we have declares that a tile exists.  Here's a 5x5 map:
~~~
tile(0,0).tile(1,0).tile(2,0).tile(3,0).tile(4,0).
tile(0,1).tile(1,1).tile(2,1).tile(3,1).tile(4,1).
tile(0,2).tile(1,2).tile(2,2).tile(3,2).tile(4,2).
tile(0,3).tile(1,3).tile(2,3).tile(3,3).tile(4,3).
tile(0,4).tile(1,4).tile(2,4).tile(3,4).tile(4,4).
~~~

Now onto the units. We'll have a term that declares that a unit exists, it's name, and who's team it is on.  And another term that says where it is. This will get regenerated each turn by the game because the positions of the units change.

We need to have unique names for the units, so we'll add a number indicating the player, followed by an index after their name (i.e. Pawn[PlayerNumber]-[PawnNumber]).

~~~
unit(King1-1, King, Player1). at(King1-1, tile(2, 4)).
unit(Pawn1-1, Pawn, Player1). at(Pawn1-1, tile(2, 3)).
unit(Pawn1-2, Pawn, Player1). at(Pawn1-2, tile(1, 4)).
unit(Pawn1-3, Pawn, Player1). at(Pawn1-3, tile(3, 4)).

unit(King2-1, King, Player2). at(King2-1, tile(2, 0)).
unit(Pawn2-1, Pawn, Player2). at(Pawn2-1, tile(2, 1)).
unit(Pawn2-2, Pawn, Player2). at(Pawn2-2, tile(1, 0)).
unit(Pawn2-3, Pawn, Player2). at(Pawn2-3, tile(3, 0)).
~~~

That's it! That is all the state that needs to be converted from the language and data structures of the game into Prolog. Now onto the HTN engine!

### HTN Methods
There are many approaches to use for breaking the problem down, but the approach I've successfully used is: Write the Methods in the way you would play the game. I'd think about this in terms of priorities, first do this, otherwise, do this, etc.

So, in English:

0. If someone is near our king, attack them.
1. If we are next to the opponent's king, attack it.
1. If there are no pawns near our king, move one back to guard it
3. If we next to an opponent's unit, attack it.
4. Otherwise, go towards the opponent's king

Let's build each Method.

#### attackKingAttackers
Our highest priority is to attack any units that are about to attack our king. Because: if we die, we don't have a chance to do anything else. In pedantic English this might be "If a unit is next to our king, attack it with any unit near it".  Note that there are better and worse units to pick based on the rest of the board layout, but we're going to keep it simple:
~~~
attackKingAttackers(?Player) :-
	if(
		% Get our king
		unit(?King, King, ?Player),
		% Get all Units around it
		unitsInRange(?King, 1, ?Unit),
		% that are from the enemy
		unit(?Unit, ?Type, ?UnitPlayer),
		\\==(?UnitPlayer, ?Player)
	),
	do(attackUnit(?Unit)).
~~~
Pretty straightforward. We have a `unitsInRange` Axiom to implement and a `attackUnit` method to implement to make this work.

##### unitsInRange
Let's start with `unitsInRange`. We need to get all the valid tiles around the king that are "in range" and return any units that are on them. This is a case where the large Internet community of Prolog programmers is very useful.  I needed a way to have Prolog generate a sequence of numbers which, it turns out, isn't completely obvious. The Internet helped:
~~~
% Prolog trick that generates a series of numbers
gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).
gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).

?- gen(0, 5, ?X).
>> ((?X = 0), (?X = 1), (?X = 2), (?X = 3), (?X = 4), (?X = 5))
~~~

So now we can generate a series of numbers, how do we turn this into a square around a particular tile?
~~~
% Prolog trick that generates a series of numbers
gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).
gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).

% hLine and vLine create a set of tiles in a line vertically or horizontally
hLineTile(?X1,?X2,?Y,tile(?S,?T)) :- gen(?X1,?X2,?S), tile(?S,?Y), is(?T,?Y).
vLineTile(?X,?Y1,?Y2,tile(?S,?T)) :- gen(?Y1,?Y2,?T), tile(?X,?T), is(?S,?X).

% Square generates a square by using the trick that Prolog 
% unifies with ALL rules, so it will get all 4 rules, each representing 
% an edge of the square
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, -(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, +(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, -(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, +(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).

?- square(tile(2,2), 2, ?Tile).
>> ((?Tile = tile(0,0)), (?Tile = tile(1,0)), (?Tile = tile(2,0)), 
(?Tile = tile(3,0)), (?Tile = tile(4,0)), (?Tile = tile(0,4)), 
(?Tile = tile(1,4)), (?Tile = tile(2,4)), (?Tile = tile(3,4)), 
(?Tile = tile(4,4)), (?Tile = tile(0,1)), (?Tile = tile(0,2)), 
(?Tile = tile(0,3)), (?Tile = tile(4,1)), (?Tile = tile(4,2)), 
(?Tile = tile(4,3)))
~~~

You'll notice that this generates the border of a square but doesn't fill it in. We need a filled square because we want a pawn anywhere inside the radius X of the king.  So:
~~~
% Prolog trick that generates a series of numbers
gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).
gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).

% hLine and vLine create a set of tiles in a line vertically or horizontally
hLineTile(?X1,?X2,?Y,tile(?S,?T)) :- gen(?X1,?X2,?S), tile(?S,?Y), is(?T,?Y).
vLineTile(?X,?Y1,?Y2,tile(?S,?T)) :- gen(?Y1,?Y2,?T), tile(?X,?T), is(?S,?X).

% Square generates a square by using the trick that Prolog 
% unifies with ALL rules, so it will get all 4 rules, each representing 
% an edge of the square
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, -(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, +(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, -(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, +(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).

filledSquare(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- 
	=<(?Min, ?Max), square(tile(?X,?Y),?Min,tile(?S,?T)).
filledSquare(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- 
	=<(?Min, ?Max), is(?Min1, +(?Min, 1)), 
	filledSquare(?Min1,?Max,tile(?X,?Y),tile(?S,?T)).

?- filledSquare(1,2, tile(2,2), ?Tile).
>> ((?Tile = tile(1,1)), (?Tile = tile(2,1)), (?Tile = tile(3,1)), 
(?Tile = tile(1,3)), (?Tile = tile(2,3)), (?Tile = tile(3,3)), 
(?Tile = tile(1,2)), (?Tile = tile(3,2)), (?Tile = tile(0,0)), 
(?Tile = tile(1,0)), (?Tile = tile(2,0)), (?Tile = tile(3,0)), 
(?Tile = tile(4,0)), (?Tile = tile(0,4)), (?Tile = tile(1,4)), 
(?Tile = tile(2,4)), (?Tile = tile(3,4)), (?Tile = tile(4,4)), 
(?Tile = tile(0,1)), (?Tile = tile(0,2)), (?Tile = tile(0,3)), 
(?Tile = tile(4,1)), (?Tile = tile(4,2)), (?Tile = tile(4,3)))
~~~
That's what we want.  Now we can implement our unitsInRange Axiom:
~~~
unitsInRange(?Unit, ?Range, ?InRangeUnit) :- 
	% Get the location of the unit
	at(?Unit, ?UnitTile),
	% Get all the tiles within ?Range squares of it
	filledSquare(1, ?Range, ?UnitTile, ?Tile),
	% Return a unit if it is on the tile
	at(?InRangeUnit, ?Tile).
~~~

##### attackUnit
To finish the `attackKingAttackers` Method:
~~~
attackKingAttackers(?Player) :-
	if(
		% Get our king
		unit(?King, King, ?Player),
		% Get all Units around it
		unitsInRange(?King, 1, ?Unit),
		% that are from the enemy
		unit(?Unit, ?Type, ?UnitPlayer),
		\\==(?UnitPlayer, ?Player)
	),
	do(attackUnit(?Unit)).
~~~
We need to implement the `attackUnit` Method.  Simplest approach here is to say "If we have a unit adjacent, attack with it":
~~~
attackUnit(?EnemyUnit) :-
	if(
		% Get the Enemy Team and position
		unit(?EnemyUnit, ?Type, ?EnemyPlayer),
		at(?EnemyUnit, ?EnemyTile),
		% Get all the units adjascent to ?EnemyUnit
		unitsInRange(?EnemyUnit, 1, ?Unit),
		% that are Units of any kind
		unit(?Unit, ?Type, ?Player),
		% on the other team
		\\==(?Player, ?EnemyPlayer),
		% Get its position
		at(?Unit, ?UnitTile)
	),
	do(captureUnit(?EnemyUnit, ?EnemyTile), doMove(?Unit, ?UnitTile, ?EnemyTile)).
~~~
In the `do()` clause are our first Operators.  This is the level where there are no more problems to break down, we just need to remove a piece and move. An operator changes the state of the world in some way.  In this case, by changing the facts about a unit's position and removing the enemy piece from the board:
~~~
% Operator that removes a piece from the map
captureUnit(?EnemyUnit, ?Tile) :- 
	del(at(?EnemyUnit, ?Tile)), add().

% Operator that actually does a move
doMove(?Unit, ?Current, ?Destination) :- 
	del(at(?Unit, ?Current)), add(at(?Unit, ?Destination)).
~~~

##### Finalizing attackUnit

So here is all the code to implement `attackUnit`.  I've modified the board so we can do some attacking!

~~~
% Layout of the map that never needs to change
tile(0,0).tile(1,0).tile(2,0).tile(3,0).tile(4,0).
tile(0,1).tile(1,1).tile(2,1).tile(3,1).tile(4,1).
tile(0,2).tile(1,2).tile(2,2).tile(3,2).tile(4,2).
tile(0,3).tile(1,3).tile(2,3).tile(3,3).tile(4,3).
tile(0,4).tile(1,4).tile(2,4).tile(3,4).tile(4,4).

% Position of units which will change each turn
unit(King1-1, King, Player1). at(King1-1, tile(2, 4)).
unit(Pawn1-1, Pawn, Player1). at(Pawn1-1, tile(2, 3)).
unit(Pawn1-2, Pawn, Player1). at(Pawn1-2, tile(1, 4)).
unit(Pawn1-3, Pawn, Player1). at(Pawn1-3, tile(3, 4)).

unit(King2-1, King, Player2). at(King2-1, tile(2, 0)).
unit(Pawn2-1, Pawn, Player2). at(Pawn2-1, tile(2, 2)).
unit(Pawn2-2, Pawn, Player2). at(Pawn2-2, tile(1, 3)).

attackUnit(?EnemyUnit) :-
	if(
		% Get the Enemy Team and position
		unit(?EnemyUnit, ?Type, ?EnemyPlayer),
		at(?EnemyUnit, ?EnemyTile),
		% Get all the units adjascent to ?EnemyUnit
		unitsInRange(?EnemyUnit, 1, ?Unit),
		% that are Units of any kind
		unit(?Unit, ?Type, ?Player),
		% on the other team
		\\==(?Player, ?EnemyPlayer),
		% Get its position
		at(?Unit, ?UnitTile)
	),
	do(captureUnit(?EnemyUnit, ?EnemyTile), doMove(?Unit, ?UnitTile, ?EnemyTile)).
	
% Operator that removes a piece from the map
captureUnit(?EnemyUnit, ?Tile) :- 
	del(at(?EnemyUnit, ?Tile)), add().

% Operator that actually does a move
doMove(?Unit, ?Current, ?Destination) :- 
	del(at(?Unit, ?Current)), add(at(?Unit, ?Destination)).

unitsInRange(?Unit, ?Range, ?InRangeUnit) :- 
	% Get the location of the unit
	at(?Unit, ?UnitTile),
	% Get all the tiles within ?Range squares of it
	filledSquare(1, ?Range, ?UnitTile, ?Tile),
	% Return a unit if it is on the tile
	at(?InRangeUnit, ?Tile).

% Prolog trick that generates a series of numbers
gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).
gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).

% hLine and vLine create a set of tiles in a line vertically or horizontally
hLineTile(?X1,?X2,?Y,tile(?S,?T)) :- gen(?X1,?X2,?S), tile(?S,?Y), is(?T,?Y).
vLineTile(?X,?Y1,?Y2,tile(?S,?T)) :- gen(?Y1,?Y2,?T), tile(?X,?T), is(?S,?X).

% Square generates a square by using the trick that Prolog 
% unifies with ALL rules, so it will get all 4 rules, each representing 
% an edge of the square
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, -(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, +(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, -(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, +(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).

filledSquare(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- 
	=<(?Min, ?Max), square(tile(?X,?Y),?Min,tile(?S,?T)).
filledSquare(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- 
	=<(?Min, ?Max), is(?Min1, +(?Min, 1)), 
	filledSquare(?Min1,?Max,tile(?X,?Y),tile(?S,?T)).

?- goals(attackUnit(King1-1)).
>> null

?- goals(attackUnit(Pawn1-1)).
>> [ { (captureUnit(Pawn1-1,tile(2,3)), doMove(Pawn2-1,tile(2,2),tile(2,3))) } 
{ (captureUnit(Pawn1-1,tile(2,3)), doMove(Pawn2-2,tile(1,3),tile(2,3))) } ]
~~~
Trying `attackUnit`with the king failed, but with Pawn1-1 it returned two different solutions because two of our pawns could attack it.  We always pick the first solution to actually use since it will usually be the best as you'll see later.

#### defendKing 
Referring back to our priorities for the AI:
0. (done!) If someone is near our king, attack them.
1. If we are next to the opponent's king, attack it.
1. If there are no pawns near our king, move one back to guard it
3. If we next to an opponent's unit, attack it.
4. Otherwise, go towards the opponent's king

Method 1 will be very analogous to Method 0, so let's implement Method 2 instead: if we don't have a king "near" the pawn, we'll move the closest one to it:
~~~
defendKing(?Player) :- 
	if(
		% Get our king
		unit(?King, King, ?Player),
		% If there are NOT any pawns around it...
		not(
			% Get all Units in Range
			unitsInRange(?King, 1, ?Unit),
			% that are pawns we own
			unit(?Unit, Pawn, ?Player)
		)
	), 
	do(moveClosestPawnNearKing(?Player, ?King)).
~~~

Notice we are using a new Axiom here called`moveClosestPawnNearKing` that we need to define now.  

#### moveClosestPawnNearKing
How do we find the closest pawn? First approximation: Get all the pawns, get their distance, sort it so we get the best ones first.
~~~
closestPawns(?Tile, ?Player, ?Pawn) :-
	% Use the built in function sortBy which takes an operator to show which way to sort
	sortBy(?Distance, <(
		% Get all of our pawns and their location
		unit(?Pawn, Pawn, ?Player), at(?Pawn, ?PawnTile),
		% Get their distance to ?Tile
		distance(?Tile, ?PawnTile, ?Distance)
	)).
~~~
Note that this will return *all* the pawns sorted from best to worst, not just the "closest" one. Shouldn't we use `first()` to just return the best? When building AI, at least, the problem with limiting the solutions like this is that the AI won't be able to search alternatives that are "2nd best" in cases where the "best" doesn't work out for some reason.  

So, I've found that it is best to build Axioms and Methods that do as many alternatives that make sense, but return them in best to worst order.

Now we just have to define `distance()`:
~~~
% Turns out distance on a grid where you move diagnally is just the
% Max of the Y or X distances...
distance(?From, ?To, ?Distance) :- 
	% Pull out the X and Y parts of ?From and ?To so we can calculate with them
	y(?From, ?FromY), x(?From, ?FromX), y(?To, ?ToY), x(?To, ?ToX), 
	% Get the difference between the Xs and the Ys
	is(?DY, abs(-(?ToY, ?FromY))), is(?DX, abs(-(?ToX, ?FromX))), 
	% The max is the distance
	is(?Distance, max(?DX, ?DY)). 

% Helper functions that return the X or Y part of a tile
x(tile(?X, ?Y), ?X) :- .
y(tile(?X, ?Y), ?Y) :- .
~~~

Let's test all this with our data:
~~~
?- closestPawns(tile(0,0), Player1, ?Which).
>> ((?Which = Pawn1-1), (?Which = Pawn1-2), (?Which = Pawn1-3))
~~~
Each of the different solutions is surrounded by `()`, so we have 3 solutions returned. OK, we have the closest pawns, in order of closeness. 

What does it mean to move a pawn "near the king"? Let's say that "near" means to the tile surrounding the king that is closest to the pawn:
~~~
closestSurroundingTilesToTile(?FromTile, ?ToTile, ?Tile) :- 
	% Get all the tiles around ?ToTile
	sortBy(?Distance, <(
		% Get all the tiles around ToTile
		square(?ToTile, 1, ?Tile),
		distance(?FromTile, ?Tile, ?Distance)
	)).

?- closestSurroundingTilesToTile(tile(0,0), tile(3,0), ?X).
>> ((?X = tile(2,1)), (?X = tile(2,0)), (?X = tile(3,1)), (?X = tile(4,1)), (?X = tile(4,0)))
~~~
Again we have just sorted the answer from best to worst so we have alternatives. You may have thought that the tile around the king that is closest to `tile(0,0)` would be `tile(2, 0)`. But since we treat diagonals as having a distance of 1, `tile(2,1)` is also correct.

So, we know which pawn, and we know where to go, but we can only move on square, so we need the best square on the path there:
~~~
% Next tile on the path from X to Y is surely the tile around X that is 
% closest to Y
nextTilesOnPath(?FromTile, ?ToTile, ?Tile) :- 
	% Sort by shortest distance
	sortBy(?Distance, <(
		% Get all the tiles around FromTile
		square(?FromTile, 1, ?Tile),
		distance(?Tile, ?ToTile, ?Distance)
	)),
	% Where the distance to the next tile is 1
	distance(?FromTile, ?Tile, ?MoveDistance),
	==(1, ?MoveDistance).
	
?- nextTilesOnPath(tile(0,0), tile(2,1), ?X).
>> ((?X = tile(1,1)), (?X = tile(1,0)), (?X = tile(0,1)))
~~~
Finally, we have everything to build our `moveClosestPawnNearKing` method:
~~~
moveClosestPawnNearKing(?Player, ?King) :- 
	if(
		% Get the position of the king
		at(?King, ?KingTile),
		% Get the closest pawns and their positions
		closestPawns(?KingTile, ?Player, ?Pawn), at(?Pawn, ?PawnTile),
		% Get unique moves sorted by distance since we have alot of alternatives 
		% and there could be duplicates
		sortBy(?Distance, <(
			distinct(?MoveTile, 
				% Figure out where we should move it
				closestSurroundingTilesToTile(?PawnTile, ?KingTile, ?ClosestTile),
				% Get the next tile on the way to ?ClosestTile
				nextTilesOnPath(?PawnTile, ?ClosestTile, ?MoveTile)
			),
			distance(?MoveTile, ?KingTile, ?Distance)
		))
	),
	% Move there
	do(tryMove(?Pawn, ?MoveTile)).
~~~
Note that, because we are returning a lot of alternatives from things like `nextTilesOnPath` and `closestSurroundingTilesToTile`, we can get duplicates, so we use the `distinct` rule to clear those out.  Also, because many of the alternatives are not the "best" alternatives, we resort it is as well.

The `do()` clause is using an operator `tryMove()` that we need to implement. This will check to make sure a move is legal since there could be a unit in that location.

#### tryMove
We can't just call an operator at this point, because there might be one of our units there or an enemy unit. So instead we call `tryMove()` which handles these cases for us
~~~
% Position of units which will change each turn
unit(King1-1, King, Player1). at(King1-1, tile(2, 4)).
unit(Pawn1-1, Pawn, Player1). at(Pawn1-1, tile(2, 3)).
unit(Pawn1-2, Pawn, Player1). at(Pawn1-2, tile(1, 4)).
unit(Pawn1-3, Pawn, Player1). at(Pawn1-3, tile(3, 4)).

unit(King2-1, King, Player2). at(King2-1, tile(2, 0)).
unit(Pawn2-1, Pawn, Player2). at(Pawn2-1, tile(2, 1)).
unit(Pawn2-2, Pawn, Player2). at(Pawn2-2, tile(1, 0)).
unit(Pawn2-3, Pawn, Player2). at(Pawn2-3, tile(3, 0)).

% Only move if it is valid
tryMove(?Unit, ?Destination) :-
	if(
		% Get current Unit position and Player
		unit(?Unit, ?UnitType, ?UnitPlayer), at(?Unit, ?Current),
		% must be only one square away
		distance(?Current, ?Destination, ?Distance), ==(1, ?Distance),
		% must not be occupied by our own team
		not(at(?BlockingUnit, ?Destination), 
			unit(?BlockingUnit, ?BlockingType, ?BlockingPlayer), 
			==(?BlockingPlayer, ?UnitPlayer)
		)
	),
	do(doMoveOrCapture(?Unit, ?Destination)).
~~~
One final piece of the puzzle: `doMoveOrCapture(?Unit, ?Destination)` is a Method that does the logic of capturing and moving or just moving.

#### doMoveOrCapture
If an enemy unit is there, we need to capture it.  Either way we need to move. That's what this method does:
~~~
% Implementation detail of tryMove, should not be called directly 
% since it hasn't been checked for validity
% Capture AND move if there is an enemy there
doMoveOrCapture(?Unit, ?Destination) :-
	if(
		at(?OtherUnit, ?Destination),
		at(?Unit, ?Current)
	),
	do(captureUnit(?OtherUnit, ?Destination), doMove(?Unit, ?Current, ?Destination)).
% otherwise just move
doMoveOrCapture(?Unit, ?Destination) :- 
	else,
	if(
		at(?Unit, ?Current)
	),
	do(doMove(?Unit, ?Current, ?Destination)).
	
% Operator that removes a piece from the map
captureUnit(?EnemyUnit, ?Tile) :- 
	del(at(?EnemyUnit, ?Tile)), add().

% Operator that actually does a move
doMove(?Unit, ?Current, ?Destination) :- 
	del(at(?Unit, ?Current)), add(at(?Unit, ?Destination)).

?- goals(tryMove(King1-1, tile(1,4))).
>> null
?- goals(tryMove(Pawn1-2, tile(1,3))).
>> [ { (doMove(Pawn1-2,tile(1,4),tile(1,3))) } ]
~~~
Moving a king to a position that is occupied fails, but moving a pawn to a position that is free succeeds and returns the "Plan", which is a list of Operators.  Now, we can tie it all together:

### Final Code For DefendKing
~~~
% Layout of the map that never needs to change
tile(0,0).tile(1,0).tile(2,0).tile(3,0).tile(4,0).
tile(0,1).tile(1,1).tile(2,1).tile(3,1).tile(4,1).
tile(0,2).tile(1,2).tile(2,2).tile(3,2).tile(4,2).
tile(0,3).tile(1,3).tile(2,3).tile(3,3).tile(4,3).
tile(0,4).tile(1,4).tile(2,4).tile(3,4).tile(4,4).

% Position of units which will change each turn
unit(King1-1, King, Player1). at(King1-1, tile(2, 4)).
unit(Pawn1-1, Pawn, Player1). at(Pawn1-1, tile(2, 3)).
unit(Pawn1-2, Pawn, Player1). at(Pawn1-2, tile(1, 4)).
unit(Pawn1-3, Pawn, Player1). at(Pawn1-3, tile(3, 4)).

unit(King2-1, King, Player2). at(King2-1, tile(2, 0)).
unit(Pawn2-1, Pawn, Player2). at(Pawn2-1, tile(2, 2)).

defendKing(?Player) :- 
	if(
		% Get our king
		unit(?King, King, ?Player),
		% If there are NOT any pawns around it...
		not(
			% Get all Units in Range
			unitsInRange(?King, 1, ?Unit),
			% that are pawns we own
			unit(?Unit, Pawn, ?Player)
		)
	), 
	do(moveClosestPawnNearKing(?Player, ?King)).

moveClosestPawnNearKing(?Player, ?King) :- 
	if(
		% Get the position of the king
		at(?King, ?KingTile),
		% Get the closest pawns and their positions
		closestPawns(?KingTile, ?Player, ?Pawn), at(?Pawn, ?PawnTile),
		% Get unique moves sorted by distance since we have alot of alternatives 
		% and there could be duplicates
		sortBy(?Distance, <(
			distinct(?MoveTile, 
				% Figure out where we should move it
				closestSurroundingTilesToTile(?PawnTile, ?KingTile, ?ClosestTile),
				% Get the next tile on the way to ?ClosestTile
				nextTilesOnPath(?PawnTile, ?ClosestTile, ?MoveTile)
			),
			distance(?MoveTile, ?KingTile, ?Distance)
		))
	),
	% Move there
	do(tryMove(?Pawn, ?MoveTile)).

% Only move if it is valid
tryMove(?Unit, ?Destination) :-
	if(
		% Get current Unit position and Player
		unit(?Unit, ?UnitType, ?UnitPlayer), at(?Unit, ?Current),
		% must be only one square away
		distance(?Current, ?Destination, ?Distance), ==(1, ?Distance),
		% must not be occupied by our own team
		not(at(?BlockingUnit, ?Destination), 
			unit(?BlockingUnit, ?BlockingType, ?BlockingPlayer), 
			==(?BlockingPlayer, ?UnitPlayer)
		)
	),
	do(doMoveOrCapture(?Unit, ?Destination)).

% Implementation detail of tryMove, should not be called directly 
% since it hasn't been checked for validity
% Capture AND move if there is an enemy there
doMoveOrCapture(?Unit, ?Destination) :-
	if(
		at(?OtherUnit, ?Destination),
		at(?Unit, ?Current)
	),
	do(captureUnit(?OtherUnit, ?Destination), doMove(?Unit, ?Current, ?Destination)).
% otherwise just move
doMoveOrCapture(?Unit, ?Destination) :- 
	else,
	if(
		at(?Unit, ?Current)
	),
	do(doMove(?Unit, ?Current, ?Destination)).

% Operator that removes a piece from the map
captureUnit(?EnemyUnit, ?Tile) :- 
	del(at(?EnemyUnit, ?Tile)), add().

% Operator that actually does a move
doMove(?Unit, ?Current, ?Destination) :- 
	del(at(?Unit, ?Current)), add(at(?Unit, ?Destination)).

% Next tile on the path from X to Y is surely the tile around X that is 
% closest to Y
nextTilesOnPath(?FromTile, ?ToTile, ?Tile) :- 
	% Sort by shortest distance
	sortBy(?Distance, <(
		% Get all the tiles around FromTile
		square(?FromTile, 1, ?Tile),
		distance(?Tile, ?ToTile, ?Distance)
	)),
	% Where the distance to the next tile is 1
	distance(?FromTile, ?Tile, ?MoveDistance),
	==(1, ?MoveDistance).

closestSurroundingTilesToTile(?FromTile, ?ToTile, ?Tile) :- 
	% Get all the tiles around ?ToTile
	sortBy(?Distance, <(
		% Get all the tiles around ToTile
		square(?ToTile, 1, ?Tile),
		distance(?FromTile, ?Tile, ?Distance)
	)).

closestPawns(?Tile, ?Player, ?Pawn) :-
	% Use the built in function sortBy which takes an operator to show which way to sort
	sortBy(?Distance, <(
		% Get all of our pawns and their location
		unit(?Pawn, Pawn, ?Player), at(?Pawn, ?PawnTile),
		% Get their distance to ?Tile
		distance(?Tile, ?PawnTile, ?Distance)
	)).

unitsInRange(?Unit, ?Range, ?InRangeUnit) :- 
	% Get the location of the unit
	at(?Unit, ?UnitTile),
	% Get all the tiles within ?Range squares of it
	filledSquare(1, ?Range, ?UnitTile, ?Tile),
	% Return a unit if it is on the tile
	at(?InRangeUnit, ?Tile).

% Turns out distance on a grid where you move diagnally is just the
% Max of the Y or X distances...
distance(?From, ?To, ?Distance) :- 
	% Pull out the X and Y parts of ?From and ?To so we can calculate with them
	y(?From, ?FromY), x(?From, ?FromX), y(?To, ?ToY), x(?To, ?ToX), 
	% Get the difference between the Xs and the Ys
	is(?DY, abs(-(?ToY, ?FromY))), is(?DX, abs(-(?ToX, ?FromX))), 
	% The max is the distance
	is(?Distance, max(?DX, ?DY)). 

% Helper functions that return the X or Y part of a tile
x(tile(?X, ?Y), ?X) :- .
y(tile(?X, ?Y), ?Y) :- .

% Prolog trick that generates a series of numbers
gen(?Cur, ?Top, ?Cur) :- =<(?Cur, ?Top).
gen(?Cur, ?Top, ?Next):- =<(?Cur, ?Top), is(?Cur1, +(?Cur, 1)), gen(?Cur1, ?Top, ?Next).

% hLine and vLine create a set of tiles in a line vertically or horizontally
hLineTile(?X1,?X2,?Y,tile(?S,?T)) :- gen(?X1,?X2,?S), tile(?S,?Y), is(?T,?Y).
vLineTile(?X,?Y1,?Y2,tile(?S,?T)) :- gen(?Y1,?Y2,?T), tile(?X,?T), is(?S,?X).

% Square generates a square by using the trick that Prolog 
% unifies with ALL rules, so it will get all 4 rules, each representing 
% an edge of the square
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, -(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?Y1, +(?Y, ?R)), is(?X1,-(?X,?R)),is(?X2, +(?X,?R)), 
	hLineTile(?X1, ?X2, ?Y1, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, -(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).
square(tile(?X,?Y),?R,tile(?S,?T)) :- 
	is(?X1, +(?X,?R)), is(?Y1,-(?Y,-(?R,1))), is(?Y2, +(?Y, -(?R,1))), 
	vLineTile(?X1, ?Y1, ?Y2, tile(?S,?T)).

filledSquare(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- 
	=<(?Min, ?Max), square(tile(?X,?Y),?Min,tile(?S,?T)).
filledSquare(?Min,?Max,tile(?X,?Y),tile(?S,?T)) :- 
	=<(?Min, ?Max), is(?Min1, +(?Min, 1)), 
	filledSquare(?Min1,?Max,tile(?X,?Y),tile(?S,?T)).

?- goals(defendKing(Player1)).
>> null

?- goals(defendKing(Player2)).
>> [ { (doMove(Pawn2-1,tile(2,2),tile(1,1))) } 
{ (doMove(Pawn2-1,tile(2,2),tile(2,1))) } 
{ (doMove(Pawn2-1,tile(2,2),tile(3,1))) } 
{ (doMove(Pawn2-1,tile(2,2),tile(1,2))) } 
{ (doMove(Pawn2-1,tile(2,2),tile(3,2))) } 
{ (captureUnit(Pawn1-1,tile(2,3)), doMove(Pawn2-1,tile(2,2),tile(2,3))) } 
{ (doMove(Pawn2-1,tile(2,2),tile(1,3))) } 
{ (doMove(Pawn2-1,tile(2,2),tile(3,3))) } ]
~~~
the `defendKing(Player1)` method fails, as expected, since it is already being defended, and returns a bunch of alternative solutions, best to worst for Player2. Note that one of the alternatives actually captures an enemy pawn, but it is far down the list because it is not really meeting the objective of being right next to the king.

### Combining defendKing and attackKingAttackers
So now we have these to top level Methods, how do we combine them to have a single AI? Since it is either one or the other, and we want to do them in priority order, we can use the `else` modifier of a method like this:
~~~
% Perform the top level AI in order of priority using 'else'
% so that only one thing happens
doAI(?Player) : - if(), do(attackKingAttackers(?Player)).
doAI(?Player) : - else, if(), do(defendKing(?Player)).
~~~
Now if we set up the world like this and run the AI:
~~~
% Position of units which will change each turn
unit(King1-1, King, Player1). at(King1-1, tile(2, 4)).
unit(Pawn1-1, Pawn, Player1). at(Pawn1-1, tile(2, 1)).
unit(King2-1, King, Player2). at(King2-1, tile(2, 0)).
unit(Pawn2-1, Pawn, Player2). at(Pawn2-1, tile(2, 2)).
unit(Pawn2-2, Pawn, Player2). at(Pawn2-2, tile(1, 3)).

?- goals(attackKingAttackers(Player2)).
>> [ { (captureUnit(Pawn1-1,tile(2,1)), doMove(Pawn2-1,tile(2,2),tile(2,1))) } ]
~~~
The AI correctly says to capture Pawn1-1 with Pawn2-1 since it was going to take our King.  

If we instead set it up like this:
~~~
% Position of units which will change each turn
unit(King1-1, King, Player1). at(King1-1, tile(2, 4)).
unit(King2-1, King, Player2). at(King2-1, tile(2, 0)).
unit(Pawn2-1, Pawn, Player2). at(Pawn2-1, tile(4, 3)).

?- goals(doAI(Player2)).
>> [ { (doMove(Pawn2-1,tile(4,3),tile(3,2))) } 
{ (doMove(Pawn2-1,tile(4,3),tile(4,2))) } 
{ (doMove(Pawn2-1,tile(4,3),tile(3,3))) } 
{ (doMove(Pawn2-1,tile(4,3),tile(3,4))) } 
{ (doMove(Pawn2-1,tile(4,3),tile(4,4))) } ]]
~~~
The AI correctly tries to move a pawn back to guard the king, and gives us a few alternatives (we'll always pick the first because it should be best).

### Everything else
Turns out the final methods are trivial given what we've already implemented:

0. (done!) If someone is near our king, attack them.
1. If we are next to the opponent's king, attack it.
2. (done!) If there are no pawns near our king, move one back to guard it
3. If we next to an opponent's unit, attack it.
4. Otherwise, go towards the opponent's king

~~~
attackKing(?Player) :-
	if(
		% Get opponent king
		opponent(?Player, ?Enemy),
		unit(?King, King, ?Enemy)
	),
	do(attackUnit(?King)).

attackOpponentUnit(?Player) :-
	if(
		%Go through each enemy unit
		opponent(?Player, ?Enemy),
		unit(?EnemyUnit, ?Type, ?Enemy)		
	),
	do(attackUnit(?EnemyUnit)).

moveTowardsOpponentKing(?Player) :-
	if(
		% Get opponent king
		opponent(?Player, ?Enemy),
		unit(?King, King, ?Enemy)
	),
	do(moveClosestPawnNearKing(?Player, ?King)).
~~~
The full code for the game AI is in the Examples directory in a file called Game.htn.


