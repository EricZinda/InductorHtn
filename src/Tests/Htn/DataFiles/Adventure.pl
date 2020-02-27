/*
:- style_check(-discontiguous).
:- dynamic(rel/3).
:- dynamic(state/2).
:- dynamic(test/1).
:- dynamic(idCounter/1).
failureContext.
failureContext(_).
failureContext(_, _).
failureContext(_, _, _).
failureContext(_, _, _, _).
failureContext(_, _, _, _, _).
count(Count, Term) :-
    aggregate_all(count, Term, Count).
*/

/***********************************/
/* Base ontology definitions       */
/***********************************/

thing.                                      % The ID for the root of all things
thing_prop_name.
rel(thing_prop_name, instanceOf, name).     % thing_prop_name is a name
rel(thing_prop_name, propertyOf, thing).    % thing_prop_name is a property of thing
state(thing_prop_name, label('thing')).     % thing_prop_name is "thing"

object.     % The ID for real objects in the world
rel(object, specializes, thing).

concept.    % The ID for concepts
rel(concept, specializes, thing).

name.       % The ID for what a human would call something
rel(name, specializes, concept).

place.
rel(place, specializes, concept).

person.
rel(person, specializes, object).


/***********************************/
/* Base ontology rules       */
/***********************************/

% specializes is
% True if the types are the same
% True if object has a sequence of specializations that lead to BaseObject
% OR if it is an instance and what it is an instance of has that
specializes(Type, Type) :- ==(Type, Type).
specializes(Type, BaseType) :-
    rel(Type, specializes, BaseType).
specializes(Type, BaseType) :-
    rel(Type, specializes, IntermediateType),
    rel(IntermediateType, specializes, BaseType).
instanceOf(Instance, BaseType) :-
    rel(Instance, instanceOf, InstanceType),
    specializes(InstanceType, BaseType).

% This is a poor mans list implementation
conj(Term) :- Term.
conj(Term, Term2) :- Term, Term2.

% ************************
% **** Event model
% ************************

idCounter(0).
getID(X) :-
    idCounter(X),
    retract(idCounter(X)),
    is(V, +(X, 1)),
    assert(idCounter(V)).
getID(_, eval) :- !.
getID(X, create) :- atomic(X), !.
getID(X, create) :- getID(X).

rel(IDSource, Relationship, IDTarget, create) :-
    getID(IDSource, create),
    getID(IDTarget, create),
    assert(rel(IDSource, Relationship, IDTarget)), !.
rel(IDSource, Relationship, IDTarget, eval) :-
    rel(IDSource, Relationship, IDTarget).
state(EntityID, Value, create) :-
    assert(state(EntityID, Value)), !.
state(EntityID, Value, eval) :-
    state(EntityID, Value).

event.      % The ID for events (i.e. things that occurr(ed))
rel(event, specializes, thing).
actor.      % The ID for an actor property of events
rel(actor, specializes, thing).
action.     % The ID for an action property of events
rel(action, specializes, thing).


% ************************
% **** Specializations for this game
% ************************

% location based relationships
% Only every pick one side of the relationship (i.e. on top / below) since the other can
% be found with not
inside.
rel(inside, specializes, place).
passageTo.
rel(passageTo, specializes, place).


% ************************
% **** Types of things that exist
% ************************
diamond.
rel(diamond, specializes, object).

rock.
rel(rock, specializes, object).

% Cave
cave.
rel(cave, specializes, place).

% Entrance
entrance.
rel(entrance, specializes, place).

% ************************
% **** Current state of the game
% ************************

% Player
player.
rel(player, instanceOf, person).
rel(player, inside, entrancecave).

% Diamond
diamond1.
rel(diamond1, instanceOf, diamond).
rel(diamond1, inside, entrancecave).

% Rocks
rock1.
rel(rock1, instanceOf, rock).
rel(rock1, inside, plage).
rock2.
rel(rock2, instanceOf, rock).
rel(rock2, inside, plage).


% World layout
entrancecave.
rel(entrancecave, instanceOf, cave).
rel(entrancecave, instanceOf, entrance).
rel(entrancecave, passageTo, plage).

plage.
rel(plage, instanceOf, cave).
rel(plage, passageTo, entrancecave).
% give it a name
rel(plage_prop_name, instanceOf, name).
rel(plage_prop_name, propertyOf, plage).
state(plage_prop_name, label('Plage')).


% At the start of the game the player went to the entrancecave
startingMovementID.
rel(startingMovementID, instanceOf, event).
startingMovement_prop_actor.
rel(startingMovement_prop_actor, instanceOf, actor).
rel(startingMovement_prop_actor, propertyOf, startingMovementID).
state(startingMovement_prop_actor, player).
startingMovement_prop_action.
rel(startingMovement_prop_action, instanceOf, action).
rel(startingMovement_prop_action, propertyOf, startingMovementID).
state(startingMovement_prop_action, go).



% ************************
% **** Delphin Predicates
% ************************

% "a person" means 1 or more things that either:
%   - thing that specializes person "is police officer a person?"
%   - thing that is instance of person "are you a person?"
d_a_q(X) :-
    failureContext(0),
    count(Count, X),
    failureContext(1, reason('no items'), X),
    not(=(Count, 0)),
    X.

d_be_v_id(_, Thing, Type, _) :-
    failureContext(0, reason('X is not a Y'), Thing, Type),
    instanceOf(Thing, Type).


d_noun(Type, X) :-
    failureContext(0, reason('X is not a Y'), X, Type),
    instanceOf(X, Type).

% X is a noun of type X if it specializes it
% as in d_noun(Person, You)
d_noun(Type, X) :-
    failureContext(0, reason('X is not a Y'), X, Type),
    specializes(X, Type).

% creating countable(_) is a hack to allow us to do count() with atoms
% "the" means the single item in question
d_the_q(X) :-
    failureContext(0),
    count(Count, X),
    failureContext(1, reason('no items'), X),
    not(=(Count, 0)),
    failureContext(2, reason('more than one item'), X),
    not(>(Count, 1)),
    X.

% Should only refer non-people
d_thing(X) :-
    rel(X, instanceOf, Kind),
    \==(Kind, person).

d_allThings(X) :-
    specializes(X, thing).

% Find the thing named Name
d_named(Name, X) :-
    rel(NamePropID, propertyOf, X),
    failureContext(1, reason('X has no names'), X),
    rel(NamePropID, instanceOf, name),
    failureContext(2, reason('X does not have name Y'), X, Name),
    state(NamePropID, label(Name)).

% What IDs to pronouns currently refer to?
d_pronoun(you, player) :- !.
d_pronoun(X, player) :-
    failureContext(1, reason('Dont know who X is'), X),
    fail.

% A location of a thing
d_in_p_loc(_, Thing, Location, _):-
    d_loc_nonsp(_, Thing, Location, _).
d_loc_nonsp(_, Thing, Location, _) :-
    % All the relationships this thing has...
    failureContext(1, reason('X has no location Y'), Thing, Location),
    rel(Thing, Rel, Location),
    % that specialize place.
    failureContext(1, reason('X has a location that is not a place'), Rel),
    specializes(Rel, place).

% Looker sees Seen
% A looker can see anything that is "in scope"
d_see_v_1(_, Looker, Seen, _) :-
    inScope(Looker, Seen).

% compound is basically describing something by saying two or more things
% that it "is": "entrance cave" is anything that is both an entrance and a cave
% item1 is a list of things that are of type1, item2 is a list of things type2
% this needs to return the intersection
d_compound(_, Item1, Item2, _) :-
    =(Item1, Item2).


% Is it true that Actor ever "goes"?
% This is a relationship between an event and an actor
% that must exist for it to be true
% start by saying that go is just an instance of an event
go.         % The ID for the go action
rel(go, specializes, action).
d_go_v_1(EventID, ActorID, CreateOrEval) :-
    rel(EventID, instanceOf, event, CreateOrEval),
    rel(ActionPropertyID, propertyOf, EventID, CreateOrEval),
        rel(ActionPropertyID, instanceOf, action, CreateOrEval),
        state(ActionPropertyID, go, CreateOrEval),
    rel(ActorPropertyID, propertyOf, EventID, CreateOrEval),
        rel(ActorPropertyID, instanceOf, actor, CreateOrEval),
        state(ActorPropertyID, ActorID, CreateOrEval).

% Preposition that ties a direction to a verbal Event
% Modeled as a property of an Event that is an instance of a location
d_to_p_dir(TermID, EventID, WhereID, CreateOrEval) :-
    rel(TermID, propertyOf, EventID, CreateOrEval),
        rel(TermID, instanceOf, place, CreateOrEval),
        state(TermID, WhereID, CreateOrEval).

% ************************
% **** Helpful rules
% ************************
%
% Anything inside what you are inside is in scope
inScope(Object, InScope) :-
    % What is Object inside?
    rel(Object, inside, ObjectInside),
    % Is InScope inside there too?
    rel(InScope, inside, ObjectInside),
    % and InScope is not the Object
    \==(Object, InScope).

