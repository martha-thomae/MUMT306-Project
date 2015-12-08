//*******************************************************************************************//
//  midiout.cpp
//  by Gary Scavone, 2003-2004.
//	Modified by Martha Thomae
//  This program outputs midi messages that generate a piece
//	in the style of the Metamorphosis works by Philip Glass.
//	It is restrained to a particular rythmic output consisting of 3 sections with a 
//	particular rhythmic pattern:
//		1. Left Hand: Quarter note against 2 eighth notes (higher in pitch)
//	while the Right Hand: bangs the root of this chord in a really low register
//		2. Left Hand: Same pattern
//	while the Right Hand: Plays the whole chord in the following rhythmic progression:
//	eigth, quarter, quarter, quarter, eigth note (so its beats doesn't coincide with 
//	the ones in the left hand)
//		3. Left Hand: Same pattern
//	while the Right Hand: Plays triplet rhythm over the notes of the left hand
//
//	The output piece choose the tonality randomly, and the same with the chord progression,
//	except that the later is restricted to just certain combinations of chords.
//	
//	So the output piece is ruled by certain instructions related to the rhythmic sections of
//	the piece, and is random with respect to the pitch of the notes that sound with these rhythms.
//*******************************************************************************************//

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort( RtMidiOut *rtmidi );

std::vector<int> ActualChord (int degree, int tonality, int baseline, int upperLimit, int lowLimit, std::vector< std::vector<int> > AllChordsInKey);

int main( void )
{
  RtMidiOut *midiout = 0;
  std::vector<unsigned char> message(3);
  int duration_eighthNote = 249;
  int delta1, delta2, delta3;
  std::vector<int> PlayingChord;
  int degree, tonality, longNote;
  std::vector< std::vector<int> > TimeSeq1;
  std::vector< std::vector<int> > TimeSeq1A(8);
  std::vector< std::vector<int> > TimeSeq1B(8);
  std::vector< std::vector<int> > TimeSeq2;
  std::vector< std::vector<int> > TimeSeq3;
//------------------------------------------------------------------//
//------------------------------------------------------------------//

// SO I RESTRICTED THE PROBLEM TO DIATONIC CHORDS (like piece # 2 in Metamorphosis)
  // THE EASIEST CASE (there is no ornamentations -notes outside the tonality-).

  // I USED A MINOR TONALITY JUST LIKE GLASS IN ALL HIS WORKS IN METAMORPHOSIS

  // AND I RESTRICTED THE CHORD PROGRESSIONS TO THE WAY HE GENERALLY MOVES BETWEEN CHORDS
  // i --> III, VI --> III, VI or iv, ii-
  // iv and ii- --> can move to themselves or to VII, v
  // And VII, v --> can move to themselves or return to i
  // I DIDN'T USE ALL THESE CASES BUT SOME OF THEM IN ORDER TO RANDOMLY HAVE 
  // DIFFERENT SEQUENCES OF CHORDS IN A PROGRESSION SIMILAR TO THIS ONE.
  // TO ACTUALLY ALLOW THIS KIND OF IMPLEMENTATION (with ALL cases allowed by Glass)
  // WE SHOULD MAKE USE OF MARKOV CHAINS, WHERE EACH NEXT CHORD HAS A WEIGHT ASSOCIATED TO IT
  
  //--------------------//
  //  CHORD PROGRESSION //
  //--------------------//

  // Vector that contains all the kinds of chord progressions allowed in the piece
  // Each vector inside it (AllChordProgressions) has numbers that indicate the scale degree of the root note of each chord.
  std::vector< std::vector<int> > AllChordProgressions(5);
  std::vector<int> chord1(1,1);
  std::vector<int> chord2(2);
  chord2[0] = 3;
  chord2[1] = 6;
  //std::vector<int> chord3(4);
  std::vector<int> chord3(2);
  chord3[0] = 3;
  chord3[1] = 6;
  //chord3[2] = 4;
  //chord3[3] = 2;
  std::vector<int> chord4(2);
  chord4[0] = 5;
  chord4[1] = 7;

  AllChordProgressions[0] = chord1;
  AllChordProgressions[1] = chord2;
  AllChordProgressions[2] = chord3;
  AllChordProgressions[3] = chord4;
  AllChordProgressions[4] = chord1;
  // Chord progression: {i} --> {III, VI} --> {III, VI, iv, ii-} --> {v, VII} --> Again {i}
  // Chord progression: {1} --> {3, 6} --> {3, 6, 4, 2} --> {5, 7} --> Again {1}


  //----------------------------------------//
  // ALL THE POSSIBLE CHORDS IN A MINOR KEY //
  //----------------------------------------//

  std::vector< std::vector<int> > AllChordsInKey(7);
  std::vector<int> chord(3);
  // i  = {root = 0, mode = minor}
  chord[0] = 0;
  chord[1] = (chord[0] + 3) % 12;
  chord[2] = (chord[0] + 7) % 12;
  AllChordsInKey[0] = chord;
  // ii-  = {root = 2, mode = diminished}
  chord[0] = 2;
  chord[1] = (chord[0] + 3) % 12;
  chord[2] = (chord[0] + 6) % 12;
  AllChordsInKey[1] = chord;
  // III  = {root = 3, mode = Major}
  chord[0] = 3;
  chord[1] = (chord[0] + 4) % 12;
  chord[2] = (chord[0] + 7) % 12;
  AllChordsInKey[2] = chord;
  // iv = {root = 5, mode = minor}
  chord[0] = 5;
  chord[1] = (chord[0] + 3) % 12;
  chord[2] = (chord[0] + 7) % 12;
  AllChordsInKey[3] = chord;
  // v  = {root = 7, mode = minor}
  chord[0] = 7;
  chord[1] = (chord[0] + 3) % 12;
  chord[2] = (chord[0] + 7) % 12;
  AllChordsInKey[4] = chord;
  // VI   = {root = 8, mode = Major}
  chord[0] = 8;
  chord[1] = (chord[0] + 4) % 12;
  chord[2] = (chord[0] + 7) % 12;
  AllChordsInKey[5] = chord;
  // VII  = {root = 10, mode = Major}
  chord[0] = 10;
  chord[1] = (chord[0] + 4) % 12;
  chord[2] = (chord[0] + 7) % 12;
  AllChordsInKey[6] = chord;
//------------------------------------------------------------------//
//------------------------------------------------------------------//

  // RtMidiOut constructor
  try {
    midiout = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }

  // Call function to select port.
  try {
    if ( chooseMidiPort( midiout ) == false ) goto cleanup;
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Send out a series of MIDI messages.
//------------------------------------------------------------------//


  //---------------------//
  // CHOOSING A TONALITY //
  //---------------------//

  srandomdev(); // seed the random number generator (every time the program runs)
  
  // Obtains a random number between 0 and 11 
  // that corresponds to one tonality, where:
  // 0 is A,    1 is A#,  2 is B,   3 is C,   4 is C#,  5 is D,
  // 6 is D#,   7 is E,   8 is F,   9 is F#,  10 is G,  11 is G#
  tonality = random()%12; 

//-----------------------------------------------------------------------------------------//

  //----------------------//
  //    FIRST PART        //
  //----------------------//----------------------------------------------//
  // This part is always in the FIRST chord (i) of the key (or tonality)  //
  // Here there are no chord progressions,                                //
  // we are always in i: AllChordsInKey[0]                                //
  // the right hand plays (when it does) the root of the chord i          //
  //----------------------------------------------------------------------//
  
  // LEFT HAND:
  // ----------
  // Generates the actual chord to be played in the limits
  PlayingChord = ActualChord (1, tonality, 45, 59, 48, AllChordsInKey);
  // For the FIRST PART we are always playing CHORD i
  // Limits in range: C3 (48) to B3 (59)
  // So its baseline (given that 0 is A) is: A2 = 45

  //ordering vector from lowest to highest value (increasing pitch)
  std::sort(PlayingChord.begin(),PlayingChord.end()); 
  // The 1st note is the lowest --> used as bass (quarter note) 
  // Last 2 notes are the highest --> used as the eighth notes (played over the quarter note)

  // RIGHT HAND:
  // -----------
  //Base line for the RIGHT HAND: A0 = 21
  longNote = 0 + tonality + 21;

  //  RYTHM:
  // -------
  for (unsigned int i = 0; i < TimeSeq1A.size(); i++){
    std::vector<int> subvector;
    // PART A
    if(i%2 == 0){
      subvector.push_back(PlayingChord[0]); //Quarter
      subvector.push_back(PlayingChord[1]); //low Eighth note
    }else{
      subvector.push_back(PlayingChord[2]); //high Eighth note
    }
    TimeSeq1A[i] = subvector;
    // PART B
    if(i == 0){
      subvector.push_back(longNote);
      subvector.push_back(longNote + 12);
    }
    TimeSeq1B[i] = subvector;
  }
  TimeSeq1.insert(TimeSeq1.end(), TimeSeq1A.begin(), TimeSeq1A.end());
  TimeSeq1.insert(TimeSeq1.end(), TimeSeq1A.begin(), TimeSeq1A.end());
  TimeSeq1.insert(TimeSeq1.end(), TimeSeq1B.begin(), TimeSeq1B.end());
  TimeSeq1.insert(TimeSeq1.end(), TimeSeq1A.begin(), TimeSeq1A.end());

  // Verification
  std::cout << tonality << std::endl;
  for (unsigned int i = 0; i < TimeSeq1.size(); i++){
    for (unsigned int j = 0; j < TimeSeq1[i].size(); j++){
      std::cout << TimeSeq1[i][j] << ", ";  
    }std::cout << std::endl;
  }
    
  // MIDI MESSAGE:
  // -------------
  delta1 = duration_eighthNote;
  // NoteOn or NoteOff
  message[0] = 144;
  // TimeSeq1 is played 2 times:
  for (unsigned int ntimes = 1; ntimes <= 2; ntimes++){
    // Velocity - NoteOn:
    message[2] = 100; //Velocity for the FIRST NOTE in the sequence
    for (unsigned int i = 0; i < TimeSeq1.size(); i++){
      // All notes that should begin at the same time:
      for (unsigned int j = 0; j < TimeSeq1[i].size(); j++){
        message[1] = TimeSeq1[i][j];
        
        //To give more volume to the notes on the RIGHT HAND 
        //(which are much more lower in the scale)
        if(j>=2){message[2] = 115;}
        
        //SEND MESSAGE (for now)
        midiout->sendMessage( &message );
      }
      SLEEP(delta1);
      message[2] = 80;
    }
  }

  //----------------------//
  //    SECOND PART       //
  //----------------------//----------------------------------//
  // This part moves from chord to chord in each measure,     //
  // so it follows the possible chord progressions defined    //
  // in the vector AllChordProgressions                       //
  // The left hand behaves just like before                   //
  // (that is why we have same code for it)                   //
  // and the right hand plays the whole chord                 //
  // (in an upper register -approx. C3 to C4-)                //
  //----------------------------------------------------------//
  for (unsigned int nmeasure = 0; nmeasure < AllChordProgressions.size(); nmeasure++){
    std::vector<int> possible_degrees = AllChordProgressions[nmeasure];
    degree = possible_degrees[random() % possible_degrees.size()];

    // LEFT HAND:
    // ----------
    // Generates the actual chord to be played in the limits
    std::vector<int> LPlayingChord = ActualChord (degree, tonality, 45, 59, 48, AllChordsInKey);
  	// For the SECOND PART we have chord progressions (degree)
  	// Limits in range: C3 (48) to B3 (59)
	// So its baseline (given that 0 is A) is: A2 = 45

    //ordering vector from lowest to highest value (increasing pitch)
    std::sort(LPlayingChord.begin(),LPlayingChord.end());     
    // The 1st note is the lowest --> used as bass (quarter note) 
    // Last 2 notes are the highest --> used as the eighth notes (played over the quarter)


    // RIGHT HAND:
    // -----------
	// Generates the actual chord to be played in the limits
    std::vector<int> RPlayingChord = ActualChord (degree, tonality, 69, 74, 60, AllChordsInKey);
	// For the SECOND PART we have chord progressions (degree)
	// Limits in range: C4 (60) to D5 (74)
	// So its baseline (given that 0 is A) is: A4 = 69
    //No need to order these notes


    //  RYTHM:
    // -------

    std::vector< std::vector<int> > TimeSeq2C(8);
    for (unsigned int i = 0; i < TimeSeq2C.size(); i++){
      std::vector<int> subvector;
      //Left hand information
      if(i%2 == 0){
        subvector.push_back(LPlayingChord[0]);  //Quarter
        subvector.push_back(LPlayingChord[1]);  //low Eighth note
      }else{
        subvector.push_back(LPlayingChord[2]);  //high Eighth note
      }
      //Right hand information
      if(i == 0 or i == 1 or i == 3 or i == 5 or i == 7){
        for (unsigned int indexNote = 0; indexNote < RPlayingChord.size(); indexNote++){
          subvector.push_back(RPlayingChord[indexNote]);
        }
      }
      TimeSeq2C[i] = subvector;
    }
    TimeSeq2.insert(TimeSeq2.end(), TimeSeq2C.begin(), TimeSeq2C.end());

    // Verification
    std::cout << "Degree: " << degree << std::endl;
    for (unsigned int i = 0; i < TimeSeq2C.size(); i++){
      for (unsigned int j = 0; j < TimeSeq2C[i].size(); j++){
        std::cout << TimeSeq2C[i][j] << ", ";  
      }std::cout << std::endl;
    }
  }

  // MIDI MESSAGE:
  // -------------
  delta2 = duration_eighthNote;
  // NoteOn or NoteOff
  message[0] = 144;
  // Velocity:
  message[2] = 80;//NoteOn
  // TimeSeq2 and TimeSeq1B (bridge section) are played 2 times:
  for (unsigned int ntimes = 1; ntimes <= 2; ntimes++){
    for (unsigned int i = 0; i < TimeSeq2.size(); i++){
      // All notes that should begin at the same time:
      for (unsigned int j = 0; j < TimeSeq2[i].size(); j++){
        message[1] = TimeSeq2[i][j];
        midiout->sendMessage( &message );
      }SLEEP(delta2);
    }// After TimeSeq2 has being played, TimeSeq1B is played to serve as a bridge between this and the next section
    for (unsigned int i = 0; i < TimeSeq1B.size(); i++){
    	for (unsigned int j = 0; j < TimeSeq1B[i].size(); j++){
    		message[1] = TimeSeq1B[i][j];
        //To give more volume to the notes on the RIGHT HAND 
        //(which are much more lower in the scale)
    		if(j>=2){message[2] = 115;}
    		midiout->sendMessage( &message );
    	}SLEEP(delta2);
    	message[2] = 80;
    }
  }

  //------------------//
  //    THIRD PART    //
  //------------------//--------------------------------------------------//
  // This part changes chord in every measure, the chord progressions   	//
  // are similar to the ones described by AllChordProgressions            //
  // The Left hand does what it has been doing all this time              //
  // (Low Quarter notes and Eighth notes moving a little higher than it)  //
  // The Right hand plays Triplets over each Eighth note.         		    //
  //----------------------------------------------------------------------//
  
  
  for (unsigned int nmeasure = 0; nmeasure < AllChordProgressions.size(); nmeasure++){
    std::vector<int> possible_degrees = AllChordProgressions[nmeasure];
    degree = possible_degrees[random() % possible_degrees.size()];

    // LEFT HAND:
    // ----------
    // Generates the actual chord to be played in the limits
    std::vector<int> LPlayingChord = ActualChord (degree, tonality, 45, 59, 48, AllChordsInKey);
    // For the THIRD PART we have chord progressions (degree)
  	// Limits in range: C3 (48) to B3 (59)
	// So its baseline (given that 0 is A) is: A2 = 45

    //ordering vector from lowest to highest value (increasing pitch)
    std::sort(LPlayingChord.begin(),LPlayingChord.end()); 
    // The 1st note is the lowest --> used as bass (quarter note) 
    // Last 2 notes are the highest --> used as the eighth notes (played over the quarter)


    // RIGHT HAND:
    // -----------
    // Generates the actual chord to be played in the limits
    std::vector<int> RPlayingChord = ActualChord (degree, tonality, 69, 76, 65, AllChordsInKey);
    // For the THIRD PART we have chord progressions (degree)
  	// Limits in range: F4 (65) to E5 (76)
	// So its baseline (given that 0 is A) is: A4 = 69
    
    //ordering vector from lowest to highest value (increasing pitch)
    std::sort(RPlayingChord.begin(),RPlayingChord.end()); 

    RPlayingChord.push_back(RPlayingChord[0] + 12);
    RPlayingChord.push_back(RPlayingChord[2]);
    RPlayingChord.push_back(RPlayingChord[1]);
    // So RPlayingChord has 6 notes, 
    // 2 triplets corresponding to each Eighth note in the Left Hand
    // It moves along the chord notes, first in an upward direction: 
    // 1st --> 2nd --> 3rd  (lowest to highest)
    // [in order to get to the 1st again but an octave higher]
    // and then it moves in the downward direction:
    // 1st (one octave higher) --> 3nd --> 2nd
    // [in order to begin again with the 1st in the original octave]


    //  RYTHM:
    // -------
    std::vector< std::vector<int> > TimeSeq3C(24);  
    // The smallest unit in the Third Part is the 
    // sixteenth note (as part of a triplet group)
    // So we divide this part in 24 sections, corresponding to:
    // 8 eighth notes * 3 triplet = 24 units on each measure.
    for (unsigned int i = 0; i < TimeSeq3C.size(); i++){
      std::vector<int> subvector;
      //Left hand information
      if(i%6 == 0){
        subvector.push_back(LPlayingChord[0]);  //Left: Quarter
        subvector.push_back(LPlayingChord[1]);  //Left: Low Eighth note
        subvector.push_back(RPlayingChord[0]);  //Right Hand (1st note)
      }else if(i%6 == 1){
        subvector.push_back(RPlayingChord[1]);  //Right Hand (2nd note)
      }else if(i%6 == 2){
        subvector.push_back(RPlayingChord[2]);  //Right Hand (3rd note)
      }else if(i%6 == 3){
        subvector.push_back(LPlayingChord[2]);  //Left: High Eighth note
        subvector.push_back(RPlayingChord[3]);  //Right Hand (4th note = 1st note an octave higher)
      }else if(i%6 == 4){
        subvector.push_back(RPlayingChord[4]);  //Right Hand (5th note)
      }else{
        subvector.push_back(RPlayingChord[5]);  //Right Hand (6th note)
      }
      TimeSeq3C[i] = subvector;
    }
    TimeSeq3.insert(TimeSeq3.end(), TimeSeq3C.begin(), TimeSeq3C.end());

    // Verification
    std::cout << "Degree: " << degree << std::endl;
    for (unsigned int i = 0; i < TimeSeq3C.size(); i++){
      for (unsigned int j = 0; j < TimeSeq3C[i].size(); j++){
        std::cout << TimeSeq3C[i][j] << ", ";  
      }std::cout << std::endl;
    }
  }

  // MIDI MESSAGE:
  // -------------
  delta3 = duration_eighthNote/3; //We are going in triplets
  // NoteOn or NoteOff
  message[0] = 144;
  // Velocity:
  message[2] = 80;//NoteOn
  // TimeSeq3 and the bridge section (which now consist of both TimeSeq1B and TimeSeq1A) are played 2 times:
  for (unsigned int ntimes = 1; ntimes <= 2; ntimes++){
    for (unsigned int i = 0; i < TimeSeq3.size(); i++){
      // All notes that should begin at the same time:
      for (unsigned int j = 0; j < TimeSeq3[i].size(); j++){
        message[1] = TimeSeq3[i][j];
        //SEND MESSAGE (for now)
        midiout->sendMessage( &message );
      }SLEEP(delta3);
    }//BRIDGE SECTION:
    for (unsigned int i = 0; i < TimeSeq1B.size(); i++){
    	for (unsigned int j = 0; j < TimeSeq1B[i].size(); j++){
    		message[1] = TimeSeq1B[i][j];
        //To give more volume to the notes on the RIGHT HAND 
        //(which are much more lower in the scale)
    		if(j>=2){message[2] = 115;}
    		midiout->sendMessage( &message );
    	}SLEEP(delta1);
    	message[2] = 80;
    }
    for (unsigned int i = 0; i < TimeSeq1A.size(); i++){
    	for (unsigned int j = 0; j < TimeSeq1A[i].size(); j++){
    		message[1] = TimeSeq1A[i][j];
    		midiout->sendMessage( &message );
    	}SLEEP(delta1);
    	message[2] = 80;
    }//ENDING OF BRIDGE SECTION.
  }

  //----------------//
  //    ENDING    	//
  //----------------//

  // MIDI MESSAGE:
  // -------------
  for (unsigned int ntimes = 1; ntimes <= 2; ntimes++){
    // Velocity - NoteOn:
    message[2] = 90; //Velocity for the FIRST NOTE in the sequence
    for (unsigned int i = 0; i < TimeSeq1.size(); i++){
      // All notes that should begin at the same time:
      for (unsigned int j = 0; j < TimeSeq1[i].size(); j++){
        message[1] = TimeSeq1[i][j];
        
        //To give more volume to the notes on the RIGHT HAND 
        //(which are much more lower in the scale)
        if(j>=2){message[2] = 115;}
        
        //SEND MESSAGE (for now)
        midiout->sendMessage( &message );
      }
      SLEEP(delta1);
      message[2] = 70;
    }
  }

//------------------------------------------------------------------//
//------------------------------------------------------------------//

  // Clean up
 cleanup:
  delete midiout;

  return 0;
}

std::vector<int> ActualChord (int degree, int tonality, int baseline, int upperLimit, int lowLimit, std::vector< std::vector<int> > AllChordsInKey){
  int note;
  std::vector<int> PlayingChord;
  for (unsigned int i = 0; i < 3; i++){
    note = AllChordsInKey[degree-1][i] + tonality + baseline;
    if(note > upperLimit){note = note - 12;}
    if(note < lowLimit){note = note + 12;}
    PlayingChord.push_back(note);
  }
  return PlayingChord;
}

bool chooseMidiPort( RtMidiOut *rtmidi )
{
  std::cout << "\nWould you like to open a virtual output port? [y/N] ";

  std::string keyHit;
  std::getline( std::cin, keyHit );
  if ( keyHit == "y" ) {
    rtmidi->openVirtualPort();
    return true;
  }

  std::string portName;
  unsigned int i = 0, nPorts = rtmidi->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No output ports available!" << std::endl;
    return false;
  }

  if ( nPorts == 1 ) {
    std::cout << "\nOpening " << rtmidi->getPortName() << std::endl;
  }
  else {
    for ( i=0; i<nPorts; i++ ) {
      portName = rtmidi->getPortName(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while ( i >= nPorts );
  }

  std::cout << "\n";
  rtmidi->openPort( i );

  return true;
}
