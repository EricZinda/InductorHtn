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

% implementation of Inductor Prolog distinct
distinct(_, Term) :-
    setof(_, Term, _).
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
idTarget.     % The ID for the target of an event
rel(idTarget, specializes, idThing).


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
    nameOf(X, _, TypeName).

% Same as d_noun but used for nouns that are of the form _n_of like world and book
d_noun(TypeName, X, _) :-
    d_noun(TypeName, X).

% creating countable(_) is a hack to allow us to do count() with atoms
% "the" means a single item, which is an *instance* of whatever got passed in
% the "instance" part is handled inside the predicate X like this:
% d_the_q(d_noun(idRock, X), isInstance(X))
d_the_q(X) :-
    failureContext(d_the_q),
    count(Count, X),
    % remove any failure context that happened during count
    % since we want the failure to be that the count is zero
    % not whatever didn't work
    failureContext(clear),
    % if count returns zero whatever caused it to fail is why there are none so we should not clear it and return that error
    % if it is >0 whatever caused it to fail just stopped us iterating, so ignore the count error
    failureContext(d_the_q, countIsZero, X),
    not(=(Count, 0)),
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

% Get means move a thing from one place and put on the idInsideFace of a free hand
idGet.
d_get_v_1(EventID, ActorID, WhatID, CreateOrEval) :-
    % Event and action
    rel(EventID, instanceOf, idEvent, CreateOrEval),
    rel(ActionPropertyID, propertyOf, EventID, CreateOrEval),
        rel(ActionPropertyID, instanceOf, idAction, CreateOrEval),
        state(ActionPropertyID, idGet, CreateOrEval),
    % Actor
    rel(ActorPropertyID, propertyOf, EventID, CreateOrEval),
        rel(ActorPropertyID, instanceOf, idActor, CreateOrEval),
        state(ActorPropertyID, ActorID, CreateOrEval),
    % What
    rel(TargetPropertyID, propertyOf, EventID, CreateOrEval),
        rel(TargetPropertyID, instanceOf, idTarget, CreateOrEval),
        state(TargetPropertyID, WhatID, CreateOrEval).

% ************************
% **** Converting objectids to english phrases
% ************************
% Describe faces by putting a preposition before them
getEnglish(FaceID, Phrase) :-
    getFace(ObjectID, FaceType, FaceID),
    getEnglish(ObjectID, ThingPhrase),
    getOrientationEnglish(FaceType, OrientationPhrase),
    phraseFromList([OrientationPhrase, ThingPhrase], Phrase), !.

getOrientationEnglish(FaceID, Phrase) :-
    member(map(FaceID, Phrase),
        [map(idLocalTop, 'on'),
        map(idLocalBottom, 'below'),
        map(idLocalLeft, 'left of'),
        map(idLocalRight, 'right of'),
        map(idLocalFront, 'in front of'),
        map(idLocalBack, 'behind'),
        map(idLocalInside, 'inside')]).

% Describe objects just by getting their names
getEnglish(ObjectID, Phrase) :-
    findall(item(InitialArticle, Name), nameOf(ObjectID, InitialArticle, Name), Chunks),
    englishFromChunks(Chunks, Phrase).

englishFromChunks([], 'an unnamed thing') :- !.

% Just a single word
englishFromChunks([item(InitialArticle, Name)], Phrase) :-
    nameOf(ObjectID, InitialArticle, Name),
    indefiniteArticle(InitialArticle, Name, Article),
    phraseFromList([Article, Name], Phrase), !.

% compound just use all the words without articles
englishFromChunks(Chunks, Phrase) :-
    removeArticles(Chunks, [], NoArticles),
    phraseFromList(NoArticles, NoArticlePhrase),
    indefiniteArticle(indefinite, NoArticlePhrase, Article),
    phraseFromList([Article, NoArticlePhrase], Phrase), !.
removeArticles([], Temp, NoArticles) :-
    reverse(Temp, NoArticles).
removeArticles([item(_, Word) | Tail], Temp, NoArticles) :-
    removeArticles(Tail, [Word | Temp], NoArticles).

% Get the right indefinite article
indefiniteArticle(indefinite, Name, Article) :-
    indefiniteArticleForName(Name, Article), !.
indefiniteArticle(false, Name, Article) :-
    indefiniteArticleForName(Name, Article), !.
% If there was an article (including '' which means none), use it
indefiniteArticle(Article, Name, Article).

indefiniteArticleForName(Name, Article) :-
    downcase_atom(Name, LowerName),
    atom_chars(LowerName, [FirstChar | _]),
    indefiniteArticleFromFirstChar(FirstChar, Article), !.

indefiniteArticleFromFirstChar(FirstChar, an) :-
    member(FirstChar, [a, e, i, o, u]), !.

indefiniteArticleFromFirstChar(FirstChar, a).


% ************************
% **** Naming
% Naming has two different rules depending on what you are looking up
% purely for performance reasons
% Article is set to '' if it is a proper name or "indefinite" if it needs an article
% ************************
nameOf(Object, Article, Name) :-
    atomic(Name),
    objectFromName(Object, Article, Name).

nameOf(Object, Article, Name) :-
    not(atomic(Name)),
    nameFromObject(Object, Article, Name).

properNameOf(Object, Name) :-
    atomic(Name),
    objectFromProperName(Object, Name).

properNameOf(Object, Name) :-
    not(atomic(Name)),
    properNameFromObject(Object, Name), !.

objectFromProperName(Object, Name) :-
    state(ProperNameProperty, label(Name)),
    rel(ProperNameProperty, instanceOf, idProperName),
    rel(ProperNameProperty, propertyOf, Object),
    isInstance(Object).

objectFromName(Object, _, Name) :-
    objectFromProperName(Object, Name).

% If it is an instance and has a common name, use that
objectFromName(Object, _, Name) :-
    state(NameProperty, label(Name)),
    rel(NameProperty, instanceOf, idName),
    rel(NameProperty, propertyOf, Object).

% Anything that derives from something with that name is one too
% return as many as exist
objectFromName(Object, _, Name) :-
    state(NameProperty, label(Name)),
    rel(NameProperty, instanceOf, idName),
    rel(NameProperty, propertyOf, BaseObject),
    instanceOf(Object, BaseObject).


% If it has a proper name use that
% and return no article since it is a proper name
properNameFromObject(Object, Name) :-
    isInstance(Object),
    rel(ProperNameProperty, propertyOf, Object),
    rel(ProperNameProperty, instanceOf, idProperName),
    state(ProperNameProperty, label(Name)).

nameFromObject(Object, Article, Name) :-
    properNameFromObject(Object, Name),
    =(Article, ''), !.

% If it is an instance and has a common name, use that
nameFromObject(Object, Article, Name) :-
    rel(NameProperty, propertyOf, Object),
    rel(NameProperty, instanceOf, idName),
    state(NameProperty, label(Name)),
    =(Article, 'indefinite'), !.

% If something it derives from has a common name, use that
% and return as many as exist
nameFromObject(Object, Article, Name) :-
    rel(Object, instanceOf, BaseType),
    nameFromObject(BaseType, Article, Name),
    =(Article, 'indefinite').

% ************************
% **** Helpful rules
% ************************
%
/*
% Iterate a list
dosomething([]).
dosomething([H|T]) :- process(H), dosomething(T).

path(X, Y) :- path(X,Y,[]).
path(X, Y, _) :- edge(X,Y).
path(X, Y, V) :- \+ member(X, V), edge(X, Z), path(Z, Y, [X|V]).

smallest([A], A).
smallest([A, B|C], D) :- A > B, smallest([B|C], D), !.
smallest([A, _|B], C) :- smallest([A|B], C).
*/
% This is a poor mans way to implement conjunction (i.e. and)
conj(Term) :- Term.
conj(Term, Term2) :- Term, Term2.

% Creates a string from a list of atoms, skipping any that are ''
phraseFromList([], _).
phraseFromList([Single], Single).
phraseFromList(['', Second | Tail], Atom) :- phraseFromList([Second | Tail], Atom), !.
phraseFromList([First, '' | Tail], Atom) :- phraseFromList([First | Tail], Atom), !.
phraseFromList([First, Second | Tail], Atom) :- atom_concat(First, ' ', SpaceFirst), atom_concat(SpaceFirst, Second, Concat), phraseFromList([Concat | Tail], Atom).

% A list is like this item(a, item())
inList(Item, bogus) :- writeln('Cant use variable as second argument of member/2'), !.
inList(Item, item(X, _)) :- ==(Item, X), !.
inList(Item, item(_, Y)) :- inList(Item, Y).

% Can move to the same place
canGo(Actor, CurrentLocation, Destination) :-
    containedIn(Actor, CurrentLocation, 1),
    ==(CurrentLocation, Destination).

% An actor can move from one container to another if sides
% are touching that have openings
canGo(Actor, CurrentLocation, Destination) :-
    containedIn(Actor, CurrentLocation, 1),
    passageTo(CurrentLocation, Destination).

% Anything inside what you are inside is in scope
inScope(Object, InScope) :-
    distinct(_, inScopeNonunique(Object, InScope)).

inScopeNonunique(Object, InScope) :-
    containedIn(Object, ObjectLocation, 1),
    containedIn(InScope, ObjectLocation, 1),
    % and InScope is not the Object
    \==(Object, InScope).

% ************************
% **** Prolog built-in predicates
% ************************

member(X, bogus) :- writeln('Cant use variable as second argument of member/2'), !.
member(X, [X|_]).
member(X, [_|Tail]) :- member(X, Tail).
reverse([],[]).
reverse([X|Xs],YsX) :- reverse(Xs,Ys), append(Ys,[X],YsX).
append([], Ys, Ys).
append([X|Xs], Ys, [X|Zs]) :- append(Xs, Ys, Zs).
% Input: merge_list([1,2],[3,4],M).
merge_list([],L,L).
merge_list([H|T],L,[H|M]):-
    merge_list(T,L,M).

% Get nth element in list match([a,b,c,d,e],2,X)
% n starts with zero
match([H|_],0,H) :- !.
match([_|T],N,H) :-
    %add for loop prevention
    >(N, 0),
    is(N1, -(N, 1)),
    match(T,N1,H).


% ************************************************************************
% ************************************************************************
% ***************************   Physics        ***************************
% ************************************************************************
% ************************************************************************

% ************************
% **** Basic bounding box
% ************************
% Physical objects can be manipulated
idPhysicalObject.
% All Physical objects are a "Place" since things can be on them, next to
% them, etc
rel(idPhysicalObject, specializes, idPlace).
idPhysicalObject_prop_name.
rel(idPhysicalObject_prop_name, instanceOf, idName).
rel(idPhysicalObject_prop_name, propertyOf, idPhysicalObject).
state(idPhysicalObject_prop_name, label('physical object')).

% ID for anything that can be "where something is"
% which always has to be a face of something in the real world
idPlace.
rel(idPlace, specializes, idConcept).
idPlace_prop_name.
rel(idPlace_prop_name, instanceOf, idName).
rel(idPlace_prop_name, propertyOf, idPlace).
state(idPlace_prop_name, label('place')).

% A physical object can have 6 faces
% each has a state (e.g. idLocalTop) which is which face they are
idFace.
rel(idFace, specializes, idPlace).

% They have seven faces which are called "local" since they don't change as the
% object is moved.  They don't have names since they are relative.
% They are used as states of idFace
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
% There is an area of Logic called Mereology (https://en.wikipedia.org/wiki/Mereology) that formally defines this
% Best description: Odell, J.J. Six different kinds of composition. Journal of Object Oriented Programming, 5 (8). 10-15.
%           http://www.cs.sjsu.edu/~pearce/modules/lectures/ooa/references/domain/compkind.html
%       Also this is a good overview:
%           https://www.w3.org/2001/sw/BestPractices/OEP/SimplePartWhole/#ref-flavours-of-part-of
%       Winston, M., Chaffin, R. and Hermann, D. A taxonomy of part-whole relations. Cognitive Science, 11. 417-444
%       Artale, A., Franconi, E. and Pazzi, L. Part-whole relations in object-centered systems: An overview. Data and Knowledge Engineering, 20. 347-383
%           http://www.inf.unibz.it/~artale/papers/appl-onto-07.pdf
%       http://www.cs.sjsu.edu/~pearce/modules/lectures/ooa/domain/agg.htm
% - What are the base concepts and how are they different?
%    - isTouching(): valid for only things that are touching
%    - compositeObject(): valid for all composite relationships
%    - Movement should move any composite objects as a unit

% "Combining" relationships: PartOf vs Touching vs Connected
%   - Describe parts of and next to
%   - All are independently nameable
%   - They differ in whether they are positioned and separable
%
% Relationships are all bidirectional
idCombiningRel.
rel(idCombiningRel, specializes, idThing).

% Positional relationships: idTouchingRel and idConnectedRel
%   - they define a face where they are
%   - Since they have a position, they are also a "Place"
idPositionalRel.
rel(idPositionalRel, specializes, idCombiningRel).
rel(idPositionalRel, specializes, idPlace).

% Composite relationships: PartOf and Connected:
%   - compose several things into one thing
%   - If you move it, it moves together
idCompositeRel.
rel(idCompositeRel, specializes, idCombiningRel).

% Touching relationships: two things that are next to each other
%   - always between two idFaces
%   - positioned, separate
idTouchingRel.
rel(idTouchingRel, specializes, idPositionalRel).

% PartOf relationship is between a part and and the thing it is part of
%   - Always between two objects
%   - unpositioned, unseparable
%   - Does not have a face connecting it, it has to have its own properties describing how it is part of
idPartOfRel.
rel(idPartOfRel, specializes, idCompositeRel).

% idConnectedRel: positioned, unseparable
%    - Must be via a face
%    - Should be one way where the minor thing is connected to the major thing
idConnectedRel.
rel(idConnectedRel, specializes, idPositionalRel).
rel(idConnectedRel, specializes, idCompositeRel).

% an Aggregate is like a Pile for things in composite relationships. It all moves together
% it returns things that are not faces
inAggregate(Object1, Object2) :-
    inAggregate(Object1, Object2, []).

inAggregate(Object1, Object2, _) :-
    connectedTo(Object1, Object2, _).

inAggregate(Object1, Object3, Visited) :-
    not(member(Object1, Visited)),
    connectedTo(Object1, Object2, _),
    inAggregate(Object2, Object3, [Object2 | Visited]),
    \==(Object1, Object3).

connectedTo(Object1, Object2, rel(Object1Face, idConnectedRel, Object2Face)) :-
    % Faces of Object 1
    rel(Object1Face, idPartOfRel, Object1),
    instanceOf(Object1Face, idFace),
    % That have an idConnectedRel relationship to another face
    rel(Object1Face, idConnectedRel, Object2Face),
    % owned by Object 2
    rel(Object2Face, idPartOfRel, Object2).

% This is the base predicate for positioned relationships
isPositioned(Object1, Object2, rel(Object1Face, PositionalRel, Object2Face)) :-
    % Faces of Object 1
    rel(Object1Face, idPartOfRel, Object1),
    instanceOf(Object1Face, idFace),
    % That have a relationship to another face
    rel(Object1Face, PositionalRel, Object2Face),
    % owned by Object 2
    rel(Object2Face, idPartOfRel, Object2),
    % That specializes idPositionalRel
    specializes(PositionalRel, idPositionalRel).

% This is the base predicate that others are built on
% isTouching means any face is touching another face that is partOf some object
isTouching(Object1, Object2, rel(Object1Face, idTouchingRel, Object2Face)) :-
    % Faces of Object 1
    rel(Object1Face, idPartOfRel, Object1),
    instanceOf(Object1Face, idFace),
    % That are touching another face
    rel(Object1Face, idTouchingRel, Object2Face),
    % owned by Object 2
    rel(Object2Face, idPartOfRel, Object2).

passageTo(CurrentLocation, Destination) :-
    isTouching(CurrentLocation, Destination, rel(CurrentFace, idTouchingRel, DestinationFace)),
    rel(CurrentOpening, idPartOfRel, CurrentFace),
    rel(CurrentOpening, instanceOf, idOpening),
    rel(DestinationOpening, idPartOfRel, DestinationFace),
    rel(DestinationOpening, instanceOf, idOpening).

onTopOf(TopObject, BottomObject):-
    isTouching(TopObject, BottomObject, rel(_, idTouchingRel, BottomFace)),
    % And the face is idLocalBottom
    state(BottomFace, idLocalTop).

% Adjacent means touching but not inside
adjacentTo(Object1, Object2, Object1Face, Object2Face) :-
    \==(Object1, Object2),
    isTouching(Object1, Object2, rel(Object1Face, idTouchingRel, Object2Face)),
    not(state(Object2Face, idLocalInside)),
    not(state(Object1Face, idLocalInside)).

% A "pile" is a set of things that are touching in any way *except* containment
% Something is in a pile if it is adjacent to it
inPileWith(Object1, Object2) :-
    inPileWith(Object1, Object2, item()).

inPileWith(Object1, Object2, _) :-
    adjacentTo(Object1, Object2, _, _).

% Something is in a pile if it is adjacent to something that is adjacent to it
inPileWith(Object1, Object3, Visited) :-
    not(inList(Object1, Visited)),
    adjacentTo(Object1, Object2, _, _),
    inPileWith(Object2, Object3, item(Object2, Visited)),
    \==(Object1, Object3), !.

getHierarchyLevelFace(Target, Level, Face) :-
    containedIn(Target, Container, Level),
    getFace(Container, idLocalInside, Face).

% True if source is contained in target, and ContainmentDepth is number of
% levels of containment (1 means directly contained)
containedIn(Source, Container, ContainmentDepth) :-
    distinct(_, containedInNonunique(Source, Container, ContainmentDepth)).

% Note that this can return duplicates!
containedInNonunique(Source, Container, ContainmentDepth) :-
    positionedNotContaining(Source, Container, ContainerFaceType, ContainmentDepth),
    state(ContainerFaceType, idLocalInside).

% True for all things the source positioned next to recursively that it is not containing
% Note that these return duplicates!
positionedNotContaining(Source, Target, TargetFaceType, ContainmentDepth) :-
    positionedNotContaining(Source, Target, TargetFaceType, ContainmentDepth, item()).
positionedNotContaining(Source, Target, TargetFaceType, ContainmentDepth, _) :-
    positionedNotContainingEdge(Source, Target, TargetFaceType, ContainmentDepth).
positionedNotContaining(Source, Target, TargetFaceType, ContainmentDepth, Visited) :-
    not(inList(Source, Visited)),
    positionedNotContainingEdge(Source, Intermediate, _, SourceDepth),
    positionedNotContaining(Intermediate, Target, TargetFaceType, IntermediateDepth, item(Source, Visited)),
    is(ContainmentDepth,  +(IntermediateDepth, SourceDepth)).

% True for all edges the source is positioned next to but not containing
% Case 1: Is a containing edge.  TargetFaceType will be idLocalInside
positionedNotContainingEdge(Source, Target, TargetFaceType, ContainmentDepth) :-
    isPositioned(Source, Target, rel(SourceFaceType, _, TargetFaceType)),
    not(state(SourceFaceType, idLocalInside)),
    state(TargetFaceType, idLocalInside),
    is(ContainmentDepth, 1).
% Case 2: Not a containing edge. TargetFaceType will NOT be idLocalInside
positionedNotContainingEdge(Source, Target, TargetFaceType, ContainmentDepth) :-
    isPositioned(Source, Target, rel(SourceFaceType, _, TargetFaceType)),
    not(state(SourceFaceType, idLocalInside)),
    not(state(TargetFaceType, idLocalInside)),
    is(ContainmentDepth, 0).

% Simplify location to: What is something contained in and what is it adjacent to
%    - Allows user to keep asking questions to get broader
% Don't things it is connected to, because then we return things like "Next to a hand" for "Where are you?"
% If the user wants that kind of location they can ask a more specific question
locationOf(Searcher, Target, Location) :-
    adjacentTo(Target, _, _, Location), !.

locationOf(Searcher, Target, Location) :-
    getHierarchyLevelFace(Target, _, Location), !.

% ************************
% **** Helpful rules
% ************************

% For now we just say that anything is moveable
% if it is a PhysicalObject
% if it is in something and has a bottom face
% If nothing is on top of it
isMoveable(What, BottomFace, TouchingFace) :-
    failureContext(isMoveable, notPhysical, What),
    instanceOf(What, idPhysicalObject),
    failureContext(isMoveable, underSomething, What),
    not(onTopOf(TopObject, What)),
    getFace(What, idLocalBottom, BottomFace),
    isTouching(What, _, rel(BottomFace, idTouchingRel, TouchingFace)).


% ************************************************************************
% ************************************************************************
% ***************************** Scenario *********************************
% ************************************************************************
% ************************************************************************
% ************************************************************************
% ************************************************************************

% ************************
% **** Specializations for this game
% ************************

% *****************************************************************
% | |                       | idCrystal1  | |                     |
% | |                       --- idSafe1 --- |                     |
% | |                           idBook1     |                     |
% | |  idLexi   idDiamond1      idTable1    |     idRock1 idRock2 |
% | |------------- idEntrancecave ----------| ------ idPlage -----|
% | -------------------------------- idWorld1 --------------------|
% ***************************************************************

% ************************
% **** Types of things that exist
% ************************
idWorld.
rel(idWorld, specializes, idPhysicalObject).
% give it a name
idWorld_prop_name.
rel(idWorld_prop_name, instanceOf, idName).
rel(idWorld_prop_name, propertyOf, idWorld).
state(idWorld_prop_name, label('world')).

idPerson.
rel(idPerson, specializes, idPhysicalObject).
idPerson_prop_name.
rel(idPerson_prop_name, instanceOf, idName).
rel(idPerson_prop_name, propertyOf, idPerson).
state(idPerson_prop_name, label('person')).

idHand.
rel(idHand, specializes, idPhysicalObject).
idHand_prop_name.
rel(idHand_prop_name, instanceOf, idName).
rel(idHand_prop_name, propertyOf, idHand).
state(idHand_prop_name, label('hand')).

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

idLockFace.
rel(idLockFace, specializes, idPhysicalObject).
% give it a name
idLockFace_prop_name.
rel(idLockFace_prop_name, instanceOf, idName).
rel(idLockFace_prop_name, propertyOf, idLockFace).
state(idLockFace_prop_name, label('lock face')).

idKeyhole.
rel(idKeyhole, specializes, idPhysicalObject).
idKeyhole_prop_name.
rel(idKeyhole_prop_name, instanceOf, idName).
rel(idKeyhole_prop_name, propertyOf, idKeyhole).
state(idKeyhole_prop_name, label('keyhole')).

idBook.
rel(idBook, specializes, idPhysicalObject).
idBook_prop_name.
rel(idBook_prop_name, instanceOf, idName).
rel(idBook_prop_name, propertyOf, idBook).
state(idBook_prop_name, label('book')).

idDiamond.
rel(idDiamond, specializes, idPhysicalObject).
% give it a name
idDiamond_prop_name.
rel(idDiamond_prop_name, instanceOf, idName).
rel(idDiamond_prop_name, propertyOf, idDiamond).
state(idDiamond_prop_name, label('diamond')).

idRock.
rel(idRock, specializes, idPhysicalObject).
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

% *****************************************************************
%                               | -----------------|
%           | -----------------|| -----------------|| -----------------|
%           |  idLexiRightHand || ---- idLexi -----|| --idLexiLeftHand-|
%           | -----------------|| -----------------|| -----------------|
%                               | -----------------|
% ***************************************************************
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
rel(idLexi, idPartOfRel, idLexiBottom).
state(idLexiBottom, idLocalBottom).
% a left face
idLexiLeft.
rel(idLexiLeft, instanceOf, idFace).
rel(idLexiLeft, idPartOfRel, idLexi).
rel(idLexi, idPartOfRel, idLexiLeft).
state(idLexiLeft, idLocalLeft).
% a right face
idLexiRight.
rel(idLexiRight, instanceOf, idFace).
rel(idLexiRight, idPartOfRel, idLexi).
rel(idLexi, idPartOfRel, idLexiRight).
state(idLexiRight, idLocalRight).

% Left hand is connected because we want it positioned and composite
idLexiLeftHand.
rel(idLexiLeftHand, instanceOf, idHand).

idLexiLeftHandBackFace.
rel(idLexiLeftHandBackFace, instanceOf, idFace).
rel(idLexiLeftHandBackFace, idPartOfRel, idLexiLeftHand).
rel(idLexiLeftHand, idPartOfRel, idLexiLeftHandBackFace).
state(idLexiLeftHandBackFace, idLocalBack).

rel(idLexiLeftHandBackFace, idConnectedRel, idLexiLeft).
rel(idLexiLeft, idConnectedRel, idLexiLeftHandBackFace).

idLexiLeftHandInsideFace.
rel(idLexiLeftHandInsideFace, instanceOf, idFace).
rel(idLexiLeftHandInsideFace, idPartOfRel, idLexiLeftHand).
rel(idLexiLeftHand, idPartOfRel, idLexiLeftHandInsideFace).
state(idLexiLeftHandInsideFace, idLocalInside).

% Right hand
idLexiRightHand.
rel(idLexiRightHand, instanceOf, idHand).

idLexiRightHandBackFace.
rel(idLexiRightHandBackFace, instanceOf, idFace).
rel(idLexiRightHandBackFace, idPartOfRel, idLexiRightHand).
rel(idLexiRightHand, idPartOfRel, idLexiRightHandBackFace).
state(idLexiRightHandBackFace, idLocalBack).

rel(idLexiRightHandBackFace, idConnectedRel, idLexiRight).
rel(idLexiRight, idConnectedRel, idLexiRightHandBackFace).

idLexiRightHandInsideFace.
rel(idLexiRightHandInsideFace, instanceOf, idFace).
rel(idLexiRightHandInsideFace, idPartOfRel, idLexiRightHand).
rel(idLexiRightHand, idPartOfRel, idLexiRightHandInsideFace).
state(idLexiRightHandInsideFace, idLocalInside).

% Location: Currently inside entrancecave
rel(idLexiBottom, idTouchingRel, idEntrancecaveInside).
rel(idEntrancecaveInside, idTouchingRel, idLexiBottom).

hand(Person, Hand) :-
    inAggregate(Person, Hand),
    instanceOf(Hand, idHand).

% Diamond
idDiamond1.
rel(idDiamond1, instanceOf, idDiamond).
% a bottom face
idDiamond1Bottom.
rel(idDiamond1Bottom, instanceOf, idFace).
rel(idDiamond1Bottom, idPartOfRel, idDiamond1).
rel(idDiamond1, idPartOfRel, idDiamond1Bottom).
state(idDiamond1Bottom, idLocalBottom).
% Current inside
rel(idDiamond1Bottom, idTouchingRel, idEntrancecaveInside).
rel(idEntrancecaveInside, idTouchingRel, idDiamond1Bottom).

% Rocks
idRock1.
rel(idRock1, instanceOf, idRock).
% a bottom face
idRock1Bottom.
rel(idRock1Bottom, instanceOf, idFace).
rel(idRock1Bottom, idPartOfRel, idRock1).
rel(idRock1, idPartOfRel, idRock1Bottom).
state(idRock1Bottom, idLocalBottom).
% Current inside
rel(idRock1Bottom, idTouchingRel, idPlageInside).
rel(idPlageInside, idTouchingRel, idRock1Bottom).

idRock2.
rel(idRock2, instanceOf, idRock).
% a bottom face
idRock2Bottom.
rel(idRock2Bottom, instanceOf, idFace).
rel(idRock2Bottom, idPartOfRel, idRock2).
rel(idRock2, idPartOfRel, idRock2Bottom).
state(idRock2Bottom, idLocalBottom).
% Current inside
rel(idRock2Bottom, idTouchingRel, idPlageInside).
rel(idPlageInside, idTouchingRel, idRock2Bottom).


% World layout
% World needs to be here so that everything is inside something
idWorld1.
rel(idWorld1, instanceOf, idWorld).
% give it an inside Face
idWorldInside.
rel(idWorldInside, instanceOf, idFace).
rel(idWorldInside, idPartOfRel, idWorld1).
rel(idWorld1, idPartOfRel, idWorldInside).
state(idWorldInside, idLocalInside).

% Entrancecave
idEntrancecave.
rel(idEntrancecave, instanceOf, idCave).
rel(idEntrancecave, instanceOf, idEntrance).
% give it an inside Face
idEntrancecaveInside.
rel(idEntrancecaveInside, instanceOf, idFace).
rel(idEntrancecaveInside, idPartOfRel, idEntrancecave).
rel(idEntrancecave, idPartOfRel, idEntrancecaveInside).
state(idEntrancecaveInside, idLocalInside).
%bottom face
idEntrancecaveBottom.
rel(idEntrancecaveBottom, instanceOf, idFace).
rel(idEntrancecaveBottom, idPartOfRel, idEntrancecave).
rel(idEntrancecave, idPartOfRel, idEntrancecaveBottom).
state(idEntrancecaveBottom, idLocalBottom).
% That is touching the world
rel(idEntrancecaveBottom, idTouchingRel, idWorldInside).
rel(idWorldInside, idTouchingRel, idEntrancecaveBottom).
%give it a back face
idEntrancecaveBack.
rel(idEntrancecaveBack, instanceOf, idFace).
rel(idEntrancecaveBack, idPartOfRel, idEntrancecave).
rel(idEntrancecave, idPartOfRel, idEntrancecaveBack).
state(idEntrancecaveBack, idLocalBack).
% That is touching plage
rel(idEntrancecaveBack, idTouchingRel, idPlageFront).
%back face has an opening
idEntrancecaveBackOpening.
rel(idEntrancecaveBackOpening, instanceOf, idOpening).
rel(idEntrancecaveBackOpening, idPartOfRel, idEntrancecaveBack).
rel(idEntrancecaveBack, idPartOfRel, idEntrancecaveBackOpening).

% Plage
idPlage.
rel(idPlage, instanceOf, idCave).
% give it a proper name
idPlage_prop_propername.
rel(idPlage_prop_propername, instanceOf, idProperName).
rel(idPlage_prop_propername, propertyOf, idPlage).
state(idPlage_prop_propername, label('Plage')).
% give it an inside Face
idPlageInside.
rel(idPlageInside, instanceOf, idFace).
rel(idPlageInside, idPartOfRel, idPlage).
rel(idPlage, idPartOfRel, idPlageInside).
state(idPlageInside, idLocalInside).
%give it a bottom face
idPlageBottom.
rel(idPlageBottom, instanceOf, idFace).
rel(idPlageBottom, idPartOfRel, idPlage).
rel(idPlage, idPartOfRel, idPlageBottom).
state(idPlageBottom, idLocalBottom).
% That is touching the world
rel(idPlageBottom, idTouchingRel, idWorldInside).
rel(idWorldInside, idTouchingRel, idPlageBottom).
%give it a front face
idPlageFront.
rel(idPlageFront, instanceOf, idFace).
rel(idPlageFront, idPartOfRel, idPlage).
rel(idPlage, idPartOfRel, idPlageFront).
state(idPlageFront, idLocalFront).
% That is touching Entrancecave
rel(idPlageFront, idTouchingRel, idEntrancecaveBack).
%front face has an opening
idPlageFrontOpening.
rel(idPlageFrontOpening, instanceOf, idOpening).
rel(idPlageFrontOpening, idPartOfRel, idPlageFront).
rel(idPlageFront, idPartOfRel, idPlageFrontOpening).


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

% Table top is touching the book bottom
idTable1Top.
rel(idTable1Top, instanceOf, idFace).
rel(idTable1Top, idPartOfRel, idTable1).
rel(idTable1, idPartOfRel, idTable1Top).
state(idTable1Top, idLocalTop).
rel(idTable1Top, idTouchingRel, idBook1Bottom).
rel(idBook1Bottom, idTouchingRel, idTable1Top).

% Table is touching the inside of the entrancecave
idTable1Bottom.
rel(idTable1Bottom, instanceOf, idFace).
rel(idTable1Bottom, idPartOfRel, idTable1).
rel(idTable1, idPartOfRel, idTable1Bottom).
state(idTable1Bottom, idLocalBottom).
rel(idTable1Bottom, idTouchingRel, idEntrancecaveInside).
rel(idEntrancecaveInside, idTouchingRel, idTable1Bottom).

% There is a book
idBook1.
rel(idBook1, instanceOf, idBook).

% Book bottom is touching table Top
idBook1Bottom.
rel(idBook1Bottom, instanceOf, idFace).
rel(idBook1Bottom, idPartOfRel, idBook1).
rel(idBook1, idPartOfRel, idBook1Bottom).
state(idBook1Bottom, idLocalBottom).

% Book top is touching safe bottom
idBook1Top.
rel(idBook1Top, instanceOf, idFace).
rel(idBook1Top, idPartOfRel, idBook1).
rel(idBook1, idPartOfRel, idBook1Top).
state(idBook1Top, idLocalTop).
rel(idBook1Top, idTouchingRel, idSafe1Bottom).
rel(idSafe1Bottom, idTouchingRel, idBook1Top).

%                                   |--------------|
%  |--------------|---------------| |--------------|
%  |- idKeyhole1 -|- idLockFace1 -| |-- idSafe1 ---|
%                                   |--------------|
%                                   |--------------|

% Keyhole1 is connected to lockface1
rel(idKeyhole1Back, idConnectedRel, idLockFace1Front).
rel(idLockFace1Front, idConnectedRel, idKeyhole1Back).
% LockFace1 connected to Safe1
rel(idLockFace1Back, idConnectedRel, idSafe1Front).
rel(idSafe1Front, idConnectedRel, idLockFace1Back).
% Safe inside is touching the crystal bottom
rel(idSafe1Inside, idTouchingRel, idCrystal1Bottom).
rel(idCrystal1Bottom, idTouchingRel, idSafe1Inside).

% There is a safe
idSafe1.
rel(idSafe1, instanceOf, idSafe).

idSafe1Inside.
rel(idSafe1Inside, instanceOf, idFace).
rel(idSafe1Inside, idPartOfRel, idSafe1).
rel(idSafe1, idPartOfRel, idSafe1Inside).
state(idSafe1Inside, idLocalInside).

idLockFace1.
rel(idLockFace1, instanceOf, idLockFace).
idLockFace1Back.
idLockFace1Front.
rel(idLockFace1Back, instanceOf, idFace).
rel(idLockFace1Back, idPartOfRel, idLockFace1).
rel(idLockFace1, idPartOfRel, idLockFace1Back).
state(idLockFace1Back, idLocalBack).

rel(idLockFace1Front, instanceOf, idFace).
rel(idLockFace1Front, idPartOfRel, idLockFace1).
rel(idLockFace1, idPartOfRel, idLockFace1Front).
state(idLockFace1Front, idLocalFront).

idSafe1Front.
rel(idSafe1Front, instanceOf, idFace).
rel(idSafe1Front, idPartOfRel, idSafe1).
rel(idSafe1, idPartOfRel, idSafe1Front).
state(idSafe1Front, idLocalFront).

idKeyhole1.
rel(idKeyhole1, instanceOf, idKeyhole).
idKeyhole1Back.
rel(idKeyhole1Back, instanceOf, idFace).
rel(idKeyhole1Back, idPartOfRel, idKeyhole1).
rel(idKeyhole1, idPartOfRel, idKeyhole1Back).
state(idKeyhole1Back, idLocalBack).

% Safe bottom is touching the book top
idSafe1Bottom.
rel(idSafe1Bottom, instanceOf, idFace).
rel(idSafe1Bottom, idPartOfRel, idSafe1).
rel(idSafe1, idPartOfRel, idSafe1Bottom).
state(idSafe1Bottom, idLocalBottom).

% There is a crystal
idCrystal1.
rel(idCrystal1, instanceOf, idCrystal).

% Crystal bottom is touching the inside of the safe
idCrystal1Bottom.
rel(idCrystal1Bottom, instanceOf, idFace).
rel(idCrystal1Bottom, idPartOfRel, idCrystal1).
rel(idCrystal1, idPartOfRel, idCrystal1Bottom).
state(idCrystal1Bottom, idLocalBottom).
