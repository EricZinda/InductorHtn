/*
% These lines are needed to run in Swish prolog

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
% Always use "id" in front of atom names so we find cases where
% we haven't given something a real name


idThing.                                      % The ID for the root of all things
idThing_prop_name.
rel(idThing_prop_name, instanceOf, idName).   % idThing_prop_name is a idName
rel(idThing_prop_name, propertyOf, idThing).  % idThing_prop_name is a property of thing
state(idThing_prop_name, label('thing')).     % idThing_prop_name is "thing"

idObject.     % The ID for real objects in the world
rel(idObject, specializes, idThing).
idObject_prop_name.
rel(idObject_prop_name, instanceOf, idName).   % idThing_prop_name is a idName
rel(idObject_prop_name, propertyOf, idObject).  % idThing_prop_name is a property of thing
state(idObject_prop_name, label('object')).     % idThing_prop_name is "thing"

idConcept.    % The ID for concepts
rel(idConcept, specializes, idThing).
idConcept_prop_name.
rel(idConcept_prop_name, instanceOf, idName).   % idThing_prop_name is a idName
rel(idConcept_prop_name, propertyOf, idConcept).  % idThing_prop_name is a property of thing
state(idConcept_prop_name, label('concept')).     % idThing_prop_name is "thing"

idName.       % The ID for what a human would call something as a common noun
rel(idName, specializes, idConcept).
idName_prop_name.
rel(idName_prop_name, instanceOf, idName).   % idThing_prop_name is a idName
rel(idName_prop_name, propertyOf, idName).  % idThing_prop_name is a property of thing
state(idName_prop_name, label('name')).     % idThing_prop_name is "thing"

idProperName.       % The ID for what a human would call something as a Proper Name
rel(idProperName, specializes, idName).
idProperName_prop_name.
rel(idProperName_prop_name, instanceOf, idName).   % idThing_prop_name is a idName
rel(idProperName_prop_name, propertyOf, idProperName).  % idThing_prop_name is a property of thing
state(idProperName_prop_name, label('proper name')).     % idThing_prop_name is "thing"

idPerson.
rel(idPerson, specializes, idObject).
idPerson_prop_name.
rel(idPerson_prop_name, instanceOf, idName).
rel(idPerson_prop_name, propertyOf, idPerson).
state(idPerson_prop_name, label('person')).


/***********************************/
/* Base ontology rules       */
/***********************************/

% X specializes Y if there is a direct specialization
specializes(Type, BaseType) :-
    rel(Type, specializes, BaseType).
% True if object has a sequence of specializations that lead to BaseObject
specializes(Type, BaseType) :-
    rel(Type, specializes, IntermediateType),
    rel(IntermediateType, specializes, BaseType).

% the straightforward interpretation
instanceOf(Instance, Type) :-
    rel(Instance, instanceOf, Type).
% An item is an instance of a Type if it is an instance of something that
% specializes type
instanceOf(Instance, BaseType) :-
    rel(Instance, instanceOf, InstanceType),
    specializes(InstanceType, BaseType).

% If something is an instance of something else, it can't also specialize something
% Useful if we are determining if X is an actual instance of something
isInstance(X) :-
    count(Count, instanceOf(X, _)),
    >(Count, 0).

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

idEvent.      % The ID for events (i.e. things that occurr(ed))
rel(idEvent, specializes, idThing).
idActor.      % The ID for an actor property of events
rel(idActor, specializes, idThing).
idAction.     % The ID for an action property of events
rel(idAction, specializes, idThing).



% ************************
% **** Delphin Predicates
% ************************

% "a person" -> a(noun(person))
% "a person" means 1 or more things that either:
%   - thing that specializes person "is a police officer a person?"
%   - thing that is instance of person "are you a person?"
d_a_q(X) :-
    failureContext(0),
    count(Count, X),
    % if count returns zero whatever caused it to fail is why there are none so we should not clear it and return that error
    failureContext(d_a_q, countIsZero, X),
    not(=(Count, 0)),
    X.

% A thing "is" another thing if it is that exact thing
d_be_v_id(_, Thing, Thing, _).
% or if it is an instance of that thing
d_be_v_id(_, Thing, OtherThing, _) :-
    failureContext(d_be_v_id1, thingIsNotOtherThing, Thing, OtherThing),
    instanceOf(Thing, OtherThing).
% or if it specializes that thing
d_be_v_id(_, Thing, OtherThing, _) :-
    failureContext(d_be_v_id2, thingIsNotOtherThing, Thing, OtherThing),
    specializes(Thing, OtherThing).

% X is a noun of type X if it specializes it
d_noun(TypeName, X) :-
    % Nouns come in the way the user said them, i.e. as a *name*
    % They need to be mapped to an ID
    failureContext(d_noun1, noItemsNamed, TypeName),
    nameOf(Type, _, TypeName),
    failureContext(d_noun1, noInstances, X, Type),
    instanceOf(X, Type).
% X is a noun of type Y if it specializes it
% as in d_noun(Person, You)
d_noun(TypeName, X) :-
    % Nouns come in the way the user said them, i.e. as a *name*
    % They need to be mapped to an ID
    failureContext(d_noun2, noItemsNamed, TypeName),
    nameOf(Type, _, TypeName),
    failureContext(d_noun2, noInstances, X, Type),
    specializes(X, Type).
% X is a noun of type Y if it actually is that Y
d_noun(TypeName, X) :-
    % Nouns come in the way the user said them, i.e. as a *name*
    % They need to be mapped to an ID
    failureContext(d_noun3, noItemsNamed, TypeName),
    nameOf(Type, _, TypeName),
    =(Type, X).

% creating countable(_) is a hack to allow us to do count() with atoms
% "the" means a single item, which is an *instance* of whatever got passed in
% the "instance" part is handled inside the predicate X like this:
% d_the_q(d_noun(idRock, X), isInstance(X))
d_the_q(X) :-
    failureContext(d_the_q),
    count(Count, X),
    % if count returns zero whatever caused it to fail is why there are none so we should not clear it and return that error
    % if it is >0 whatever caused it to fail just stopped us iterating, so ignore the count error
    failureContext(d_the_q, countIsZero, X),
    not(=(Count, 0)),
    % remove any failure context that happened during count
    failureContext(clear),
    failureContext(d_the_q, moreThanOneItem, X),
    not(>(Count, 1)),
    X.

% Everything!
d_thing(X) :-
    rel(X, instanceOf, _).

d_thing(X) :-
    rel(X, specializes, _).

d_allThings(X) :-
    specializes(X, idThing).

% Find the thing named Name
d_named(Name, X) :-
    failureContext(d_named, notNamed, X, Name),
    properNameOf(X, Name).

% What IDs to pronouns currently refer to?
defaultActor(idLexi).
d_pronoun(you, X) :- defaultActor(X), !.
d_pronoun(X, Y) :- defaultActor(Y),
    failureContext(d_pronoun, dontKnowPronounReference, X),
    fail.

% A location of a thing, which is always described in relation to a
% "default actor"
d_in_p_loc(_, Thing, Location, _):-
    d_loc_nonsp(_, Thing, Location, _).
d_loc_nonsp(_, Thing, Location, _) :-
    failureContext(d_loc_nonsp, thingHasNoLocation, Thing, Location),
    defaultActor(Actor),
    locationOf(Actor, Thing, Location).

% Looker sees Seen
% A looker can see anything that is "in scope"
d_see_v_1(_, Looker, Seen, _) :-
    inScope(Looker, Seen).

% compound is basically describing something by saying two or more things
% that it "is": "entrance cave" is anything that is both an entrance and a cave
% item1 is a list of things that are of type1, item2 is a list of things type2
% this needs to all items that are both
d_compound(_, Item1, Item2, _) :-
    failureContext(d_compound, noThingThatIsBoth, Item1, Item2),
    =(Item1, Item2).

% Is it true that Actor ever "goes"?
% This is a relationship between an event and an actor
% that must exist for it to be true
% start by saying that go is just an instance of an event
idGo.         % The ID for the go action
rel(idGo, specializes, idAction).
d_go_v_1(EventID, ActorID, CreateOrEval) :-
    rel(EventID, instanceOf, idEvent, CreateOrEval),
    rel(ActionPropertyID, propertyOf, EventID, CreateOrEval),
        rel(ActionPropertyID, instanceOf, idAction, CreateOrEval),
        state(ActionPropertyID, idGo, CreateOrEval),
    rel(ActorPropertyID, propertyOf, EventID, CreateOrEval),
        rel(ActorPropertyID, instanceOf, idActor, CreateOrEval),
        state(ActorPropertyID, ActorID, CreateOrEval).

% Preposition that ties a direction to a verbal Event
% Modeled as a property of an Event that is an instance of a location
d_to_p_dir(TermID, EventID, WhereID, CreateOrEval) :-
    rel(TermID, propertyOf, EventID, CreateOrEval),
        rel(TermID, instanceOf, idPlace, CreateOrEval),
        state(TermID, WhereID, CreateOrEval).

% ************************
% **** Helpful rules
% ************************
%
% An actor can move from on container to another if sides
% are touching that have openings
canGo(Actor, CurrentLocation, Destination) :-
    containedIn(Actor, CurrentLocation, 1),
    passageTo(CurrentLocation, Destination).

% If it has a proper name use that
% and return no article since it is a proper name
properNameOf(Object, Name) :-
    isInstance(Object),
    rel(ProperNameProperty, propertyOf, Object),
    rel(ProperNameProperty, instanceOf, idProperName),
    state(ProperNameProperty, label(Name)).

nameOf(Object, Article, Name) :-
    properNameOf(Object, Name),
    =(Article, ''), !.

% If it is an instance and has a common name, use that
nameOf(Object, Article, Name) :-
    rel(NameProperty, propertyOf, Object),
    rel(NameProperty, instanceOf, idName),
    state(NameProperty, label(Name)),
    =(Article, 'indefinite'), !.

% If something it derives from has a common name, use that
% and return as many as exist
nameOf(Object, Article, Name) :-
    rel(Object, instanceOf, BaseType),
    nameOf(BaseType, Article, Name),
    =(Article, 'indefinite').

% Anything inside what you are inside is in scope
inScope(Object, InScope) :-
    distinct(_, inScopeNonunique(Object, InScope)).

inScopeNonunique(Object, InScope) :-
    containedIn(Object, ObjectLocation, 1),
    containedIn(InScope, ObjectLocation, 1),
    % and InScope is not the Object
    \==(Object, InScope).


    


% ************************
% **** Basic bounding box
% ************************
% Physical objects can be manipulated
idPhysicalObject.
% All Physical objects are a "Place" since things can be on them, next to
% them, etc
rel(idPhysicalObject, specializes, idObject).
idPhysicalObject_prop_name.
rel(idPhysicalObject_prop_name, instanceOf, idName).
rel(idPhysicalObject_prop_name, propertyOf, idPhysicalObject).
state(idPhysicalObject, label('physical object')).

% ID for anything that can be "where something is"
% which always has to be a face of something in the real world
idPlace.
rel(idPlace, specializes, idConcept).
idPlace_prop_name.
rel(idPlace_prop_name, instanceOf, idName).
rel(idPlace_prop_name, propertyOf, idPlace).
state(idPlace_prop_name, label('place')).

% A physical object can have 6 faces
% each has a state which is which face they are
% They are all places where something can be located
idFace.
rel(idFace, specializes, idPlace).

% They have seven faces which are called "local" since they don't change as the
% object is moved.  They don't have names since they are relative
idLocalTop.
idLocalBottom.
idLocalLeft.
idLocalRight.
idLocalFront.
idLocalBack.
idLocalInside.

% A face can have an opening which is a partOf that face
idOpening.
rel(idOpening, specializes, idPlace).

getFace(Object, FaceType, Face) :-
    rel(Face, idPartOfRel, Object),
    rel(Face, instanceOf, idFace),
    state(Face, FaceType).

% ************************
% **** Composition Model
% ************************
idCompositionRel.
rel(idCompositionRel, specializes, idPlace).

% Relationships are unidirectional, so you need to say each side is idTouching
% the other
% A Touching relationship is always between two idFaces
idTouchingRel.
rel(idTouchingRel, specializes, idCompositionRel).

% A partOf relationship is between a part and and the thing it is part of
idPartOfRel.
rel(idPartOfRel, specializes, idCompositionRel).

passageTo(CurrentLocation, Destination) :-
    isTouching(CurrentLocation, Destination, rel(CurrentFace, idTouchingRel, DestinationFace)),
    rel(CurrentOpening, idPartOfRel, CurrentFace),
    rel(CurrentOpening, instanceOf, idOpening),
    rel(DestinationOpening, idPartOfRel, DestinationFace),
    rel(DestinationOpening, instanceOf, idOpening).

onTopOf(TopObject, BottomObject):-
    isTouching(TopObject, BottomObject, rel(Object1Face, idTouchingRel, _)),
    % And the face is idLocalBottom
    state(Object1Face, idLocalBottom).

% isTouching means any face is touching
isTouching(Object1, Object2, rel(Object1Face, idTouchingRel, Object2Face)) :-
    % Faces of Object 1
    rel(Object1Face, idPartOfRel, Object1),
    instanceOf(Object1Face, idFace),
    % That are touching another face
    rel(Object1Face, idTouchingRel, Object2Face),
    % owned by Object 2
    rel(Object2Face, idPartOfRel, Object2).

% Adjacent means touching but not inside
adjacentTo(Object1, Object2) :-
    \==(Object1, Object2),
    isTouching(Object1, Object2, rel(Object1Face, idTouchingRel, Object2Face)),
    not(state(Object2Face, idLocalInside)),
    not(state(Object1Face, idLocalInside)).

% A "pile" is a set of things that are touching in any way *except* containment
% Something is in a pile if it is adjacent to it
inPileWith(Object1, Object2) :-
    adjacentTo(Object1, Object2).

% Something is in a pile if it is adjacent to something that is adjacent to it
inPileWith(Object1, Object3) :-
    adjacentTo(Object1, Object2),
    inPileWith(Object2, Object3),
    \==(Object1, Object3), !.

% A list is like this item(a, item())
inList(Item, item(X, _)) :- ==(Item, X), !.
inList(Item, item(_, Y)) :- inList(Item, Y).

% True if source is contained in target, and ContainmentDepth is number of
% levels of containment (1 means directly contained)
containedIn(Source, Container, ContainmentDepth) :-
    distinct(_, containedInNonunique(Source, Container, ContainmentDepth)).

% Note that this can return duplicates!
containedInNonunique(Source, Container, ContainmentDepth) :-
    pileOrContaining(Source, Container, ContainerFaceType, ContainmentDepth),
    state(ContainerFaceType, idLocalInside).

% True for all things the source is touching rec that it is not containing
% Note that these return duplicates!
pileOrContaining(Source, Target, TargetFaceType, ContainmentDepth) :-
    pileOrContaining(Source, Target, TargetFaceType, ContainmentDepth, item()).
pileOrContaining(Source, Target, TargetFaceType, ContainmentDepth, _) :-
    pileOrContainingEdge(Source, Target, TargetFaceType, ContainmentDepth).
pileOrContaining(Source, Target, TargetFaceType, ContainmentDepth, Visited) :-
    not(inList(Source, Visited)),
    pileOrContainingEdge(Source, Intermediate, _, SourceDepth),
    pileOrContaining(Intermediate, Target, TargetFaceType, IntermediateDepth, item(Source, Visited)),
    is(ContainmentDepth,  +(IntermediateDepth, SourceDepth)).

% True for all edges the source is touching that it is not containing
pileOrContainingEdge(Source, Target, TargetFaceType, ContainmentDepth) :-
    isTouching(Source, Target, rel(SourceFaceType, idTouchingRel, TargetFaceType)),
    not(state(SourceFaceType, idLocalInside)),
    state(TargetFaceType, idLocalInside),
    is(ContainmentDepth, 1).
pileOrContainingEdge(Source, Target, TargetFaceType, ContainmentDepth) :-
    isTouching(Source, Target, rel(SourceFaceType, idTouchingRel, TargetFaceType)),
    not(state(SourceFaceType, idLocalInside)),
    not(state(TargetFaceType, idLocalInside)),
    is(ContainmentDepth, 0).

/*
 * Original Idea:
path(X, Y) :- path(X,Y,[]).

path(X, Y, _) :- edge(X,Y).
path(X, Y, V) :- \+ member(X, V), edge(X, Z), path(Z, Y, [X|V]).
*/

% How to deal with the human concept of "Location".
% It will always return either a specific thing the object is touching in some way
% OR it will return an object that it is eventually inside (which always has an inside)
% First, we always assume the user knows the "most obvious thing" which is what is in the
% same container as them, so we give them one more level of detail if we have it
%
% it is always relative to the user. If the user is asking for location
% they must not know, so give them the next level of detail from what they should know
% - At the highest level, the location of a thing is the outermost thing it is inside
% - If the human and the thing are inside the same thing but at different ultimate levels,
%   it is the next level of inside that doesn't include the player
% - if they are literally in the same "inside", then
%   the player must be asking about what it is next to so get the "touching" object
% - if they are literally in the same "inside", and the thing is not touching anything except
%   the inside then return that inside

% If Searcher and thing are the same, location is the top level container
locationOf(Searcher, Thing, Location) :-
    ==(Searcher, Thing),
    containedIn(Searcher, Location, 1), !.

% Note that this can return multiple duplicate locations!
locationOf(Searcher, Thing, Location) :-
    containedIn(Searcher, SharedLocation, _),
    containedIn(Thing, SharedLocation, _),
    \==(Searcher, Thing),
    describeLocationOf(Searcher, Thing, Location).


% If Searcher is in immediate level X and Thing eventually shares that level
% Return the level that is one closer to Thing
describeLocationOf(Searcher, Thing, Location) :-
    containedIn(Searcher, SearcherLocation, 1),
    containedIn(Thing, SearcherLocation, ThingLevel),
    \==(ThingLevel, 1),
    is(ThingLocationLevel, -(ThingLevel, 1)),
    containedIn(Thing, Location, ThingLocationLevel), !.
% Or vice versa:
% - Lexi in Cave1 which is in the world
% - Cave 2 is in the world
% Where is cave 2?
describeLocationOf(Searcher, Thing, Location) :-
    containedIn(Searcher, SearcherLocation, SearcherLevel),
    \==(SearcherLevel, 1),
    containedIn(Thing, SearcherLocation, 1),
    is(SearcherLocationLevel, -(SearcherLevel, 1)),
    containedIn(Thing, Location, SearcherLocationLevel), !.

% If Searcher and Thing are directly inside the same container
% and thing is touching something besides the container, return that
% because presumably the Searcher knows what is around them
describeLocationOf(Searcher, Thing, Location) :-
    containedIn(Searcher, ContainLocation, 1),
    containedIn(Thing, ContainLocation, 1),
    isTouching(Thing, _, rel(SourceFaceType, idTouchingRel, Location)),
    not(state(SourceFaceType, idLocalInside)),
    not(state(Location, idLocalInside)), !.

% Otherwise if Searcher and Thing are directly inside the same container
% and thing is not touching anything besides the container, return the container
% because presumably the Searcher knows what is around them but there is nothing
% more descriptive to return
describeLocationOf(Searcher, Thing, Location) :-
    containedIn(Searcher, Location, 1),
    containedIn(Thing, Location, 1).




% ************************
% **** Specializations for this game
% ************************


% *****************************************************************
% **** Scenario: Table with book.  safe on book containing crystals
%
% | |   | Crystal1  | |                     |
% | |   --- Safe ---  |                     |
% | |       Book      |                     |
% | |       Table     |                     |
% | - Entrance Cave - | ------ Plage ------ |
% | --------------- World ------------------|
% ************************


% ************************
% **** Types of things that exist
% ************************
idWorld.
rel(idWorld, specializes, idObject).
% give it a name
idWorld_prop_name.
rel(idWorld_prop_name, instanceOf, idName).
rel(idWorld_prop_name, propertyOf, idWorld).
state(idWorld_prop_name, label('world')).

idCrystal.
rel(idCrystal, specializes, idPhysicalObject).
idCrystal_prop_name.
rel(idCrystal_prop_name, instanceOf, idName).
rel(idCrystal_prop_name, propertyOf, idCrystal).
state(idCrystal_prop_name, label('crystal')).

idTable.
rel(idTable, specializes, idPhysicalObject).
idTable_prop_name.
rel(idTable_prop_name, instanceOf, idName).
rel(idTable_prop_name, propertyOf, idTable).
state(idTable_prop_name, label('table')).

idSafe.
rel(idSafe, specializes, idPhysicalObject).
idSafe_prop_name.
rel(idSafe_prop_name, instanceOf, idName).
rel(idSafe_prop_name, propertyOf, idSafe).
state(idSafe_prop_name, label('safe')).

idBook.
rel(idBook, specializes, idPhysicalObject).
idBook_prop_name.
rel(idBook_prop_name, instanceOf, idName).
rel(idBook_prop_name, propertyOf, idBook).
state(idBook_prop_name, label('book')).

idDiamond.
rel(idDiamond, specializes, idObject).
% give it a name
idDiamond_prop_name.
rel(idDiamond_prop_name, instanceOf, idName).
rel(idDiamond_prop_name, propertyOf, idDiamond).
state(idDiamond_prop_name, label('diamond')).

idRock.
rel(idRock, specializes, idObject).
% give it a name
idRock_prop_name.
rel(idRock_prop_name, instanceOf, idName).
rel(idRock_prop_name, propertyOf, idRock).
state(idRock_prop_name, label('rock')).

% Cave
idCave.
rel(idCave, specializes, idPlace).
% give it a name
idCave_prop_name.
rel(idCave_prop_name, instanceOf, idName).
rel(idCave_prop_name, propertyOf, idCave).
state(idCave_prop_name, label('cave')).

% Entrance
idEntrance.
rel(idEntrance, specializes, idPlace).
% give it a name
idEntrance_prop_name.
rel(idEntrance_prop_name, instanceOf, idName).
rel(idEntrance_prop_name, propertyOf, idEntrance).
state(idEntrance_prop_name, label('entrance')).

% Just need an example of another person type
idFireman.
rel(idFireman, specializes, idPerson).
% give it a name
idFireman_prop_name.
rel(idFireman_prop_name, instanceOf, idName).
rel(idFireman_prop_name, propertyOf, idFireman).
state(idFireman_prop_name, label('fireman')).

% ************************
% **** Current state of the game
% ************************

% Lexi
idLexi.
rel(idLexi, instanceOf, idPerson).
% give Lexi a proper name
idLexi_prop_propername.
rel(idLexi_prop_propername, instanceOf, idProperName).
rel(idLexi_prop_propername, propertyOf, idLexi).
state(idLexi_prop_propername, label('Lexi')).
% a bottom face
idLexiBottom.
rel(idLexiBottom, instanceOf, idFace).
rel(idLexiBottom, idPartOfRel, idLexi).
state(idLexiBottom, idLocalBottom).
% Current inside
rel(idLexiBottom, idTouchingRel, idEntrancecaveInside).
rel(idEntrancecaveInside, idTouchingRel, idLexiBottom).

% Diamond
idDiamond1.
rel(idDiamond1, instanceOf, idDiamond).
% a bottom face
idDiamond1Bottom.
rel(idDiamond1Bottom, instanceOf, idFace).
rel(idDiamond1Bottom, idPartOfRel, idDiamond1).
state(idDiamond1Bottom, idLocalBottom).
% Current inside
rel(idDiamond1Bottom, idTouchingRel, idEntrancecaveInside).

% Rocks
idRock1.
rel(idRock1, instanceOf, idRock).
% a bottom face
idRock1Bottom.
rel(idRock1Bottom, instanceOf, idFace).
rel(idRock1Bottom, idPartOfRel, idRock1).
state(idRock1Bottom, idLocalBottom).
% Current inside
rel(idRock1Bottom, idTouchingRel, idPlageInside).

idRock2.
rel(idRock2, instanceOf, idRock).
% a bottom face
idRock2Bottom.
rel(idRock2Bottom, instanceOf, idFace).
rel(idRock2Bottom, idPartOfRel, idRock2).
state(idRock2Bottom, idLocalBottom).
% Current inside
rel(idRock2Bottom, idTouchingRel, idPlageInside).


% World layout
% World needs to be here so that everything is inside something
idWorld1.
rel(idWorld, instanceOf, idWorld).
% give it an inside Face
idWorldInside.
rel(idWorldInside, instanceOf, idFace).
rel(idWorldInside, idPartOfRel, idWorld1).
state(idWorldInside, idLocalInside).

% Entrancecave
idEntrancecave.
rel(idEntrancecave, instanceOf, idCave).
rel(idEntrancecave, instanceOf, idEntrance).
% give it an inside Face
idEntrancecaveInside.
rel(idEntrancecaveInside, instanceOf, idFace).
rel(idEntrancecaveInside, idPartOfRel, idEntrancecave).
state(idEntrancecaveInside, idLocalInside).
%bottom face
idEntrancecaveBottom.
rel(idEntrancecaveBottom, instanceOf, idFace).
rel(idEntrancecaveBottom, idPartOfRel, idEntrancecave).
state(idEntrancecaveBottom, idLocalBottom).
% That is touching the world
rel(idEntrancecaveBottom, idTouchingRel, idWorldInside).
%give it a back face
idEntrancecaveBack.
rel(idEntrancecaveBack, instanceOf, idFace).
rel(idEntrancecaveBack, idPartOfRel, idEntrancecave).
state(idEntrancecaveBack, idLocalBack).
%back face has an opening
idEntrancecaveBackOpening.
rel(idEntrancecaveBackOpening, instanceOf, idOpening).
rel(idEntrancecaveBackOpening, idPartOfRel, idEntrancecaveBack).
% That is touching plage
rel(idEntrancecaveBack, idTouchingRel, idPlageFront).

% Plage
idPlage.
rel(idPlage, instanceOf, idCave).
% give it a proper name
idPlage_prop_propernameage.
rel(idPlage_prop_propernameage, instanceOf, idProperName).
rel(idPlage_prop_propernameage, propertyOf, idPlage).
state(idPlage_prop_propernameage, label('Plage')).
% give it an inside Face
idPlageInside.
rel(idPlageInside, instanceOf, idFace).
rel(idPlageInside, idPartOfRel, idPlage).
state(idPlageInside, idLocalInside).
%give it a bottom face
idPlageBottom.
rel(idPlageBottom, instanceOf, idFace).
rel(idPlageBottom, idPartOfRel, idPlage).
state(idPlageBottom, idLocalBottom).
% That is touching the world
rel(idPlageBottom, idTouchingRel, idWorldInside).
%give it a front face
idPlageFront.
rel(idPlageFront, instanceOf, idFace).
rel(idPlageFront, idPartOfRel, idPlage).
state(idPlageFront, idLocalFront).
%front face has an opening
idPlageFrontOpening.
rel(idPlageFrontOpening, instanceOf, idOpening).
rel(idPlageFrontOpening, idPartOfRel, idPlageFront).
% That is touching Entrancecave
rel(idPlageFront, idTouchingRel, idEntrancecaveBack).


% At the start of the game lexi went to the entrancecave
idStartingMovement.
rel(idStartingMovement, instanceOf, idEvent).
idStartingMovement_prop_actor.
rel(idStartingMovement_prop_actor, instanceOf, idActor).
rel(idStartingMovement_prop_actor, propertyOf, idStartingMovement).
state(idStartingMovement_prop_actor, idLexi).
idStartingMovement_prop_action.
rel(idStartingMovement_prop_action, instanceOf, idAction).
rel(idStartingMovement_prop_action, propertyOf, idStartingMovement).
state(idStartingMovement_prop_action, idGo).

% There is a table
idTable1.
rel(idTable1, instanceOf, idTable).

% Table top is touching the safe bottom
idTable1Top.
rel(idTable1Top, instanceOf, idFace).
rel(idTable1Top, idPartOfRel, idTable1).
state(idTable1Top, idLocalTop).
rel(idTable1Top, idTouchingRel, idBook1Bottom).

% Table is touching the inside of the entrancecave
idTable1Bottom.
rel(idTable1Bottom, instanceOf, idFace).
rel(idTable1Bottom, idPartOfRel, idTable1).
state(idTable1Bottom, idLocalBottom).
rel(idTable1Bottom, idTouchingRel, idEntrancecaveInside).

% There is a book
idBook1.
rel(idBook1, instanceOf, idBook).

% Book bottom is touching table Top
idBook1Bottom.
rel(idBook1Bottom, instanceOf, idFace).
rel(idBook1Bottom, idPartOfRel, idBook1).
state(idBook1Bottom, idLocalBottom).
rel(idBook1Bottom, idTouchingRel, idTable1Top).

% Book top is touching safe bottom
idBook1Top.
rel(idBook1Top, instanceOf, idFace).
rel(idBook1Top, idPartOfRel, idBook1).
state(idBook1Top, idLocalTop).
rel(idBook1Top, idTouchingRel, idSafe1Bottom).


% There is a safe
idSafe1.
rel(idSafe1, instanceOf, idSafe).

% Safe inside is touching the crystal bottom
idSafe1Inside.
rel(idSafe1Inside, instanceOf, idFace).
rel(idSafe1Inside, idPartOfRel, idSafe1).
state(idSafe1Inside, idLocalInside).
rel(idSafe1Inside, idTouchingRel, idCrystal1Bottom).

% Safe bottom is touching the book top
idSafe1Bottom.
rel(idSafe1Bottom, instanceOf, idFace).
rel(idSafe1Bottom, idPartOfRel, idSafe1).
state(idSafe1Bottom, idLocalBottom).
rel(idSafe1Bottom, idTouchingRel, idBook1Top).

% There is a crystal
idCrystal1.
rel(idCrystal1, instanceOf, idCrystal).

% Crystal bottom is touching the inside of the safe
idCrystal1Bottom.
rel(idCrystal1Bottom, instanceOf, idFace).
rel(idCrystal1Bottom, idPartOfRel, idCrystal1).
state(idCrystal1Bottom, idLocalBottom).
rel(idCrystal1Bottom, idTouchingRel, idSafe1Inside).