static const struct _stationList {
  const char    *name;  // Noame of the stream
  const char    *url;   // URL of stream server
  unsigned int  volume; // The default start volume
  bool          mp3;    // mp3 or AAC
} stationList[] = 
{
  // http://pe1chl.nl/repeaters/
  { "PI2NOS 430.125"  , "http://stream.hobbyscoop.nl/pi2nos"              , 1, true },   // 00
  { "PI3UTR 145.275"  , "http://stream.hobbyscoop.nl/pi3utr"              , 33, true },   // 01
  { "PI6NOS 1298.375" , "http://stream.hobbyscoop.nl/pi6nos"              , 33, true },   // 02
  { "PI2NON 430.275"  , "http://pc7x.net/audio/pi2non/"                   , 33, true },
  { "PI3GOE 145.755"  , "http://stream.hobbyscoop.nl/pi3goe"              , 33, true },
  { "PI1DFT 430.087"  , "http://stream.hobbyscoop.nl/pi1dft"              , 33, true },
  { "PI1ZLD 438.400"  , "http://stream.hobbyscoop.nl/pi1zld"              , 33, true },

  // https://www.hendrikjansen.nl/henk/streaming.html
  //  -sb-  96 Kbps   12 KB/s
  //  -bb-  192 Kbps  24 KB/s

// http://icecast.omroep.nl/radio5-sb-aac
//  { "NPO Radio 5"     , "http://icecast.omroep.nl/radio5-sb-mp3"          },
// http://www.mp3streams.nl/zender/npo-radio-5/stream/6-aac-64

  { "NPO Radio 1"     , "http://icecast.omroep.nl/radio1-sb-mp3"          , 50, true },     // 07
  { "NPO Radio 2"     , "http://icecast.omroep.nl/radio2-sb-mp3"          , 33, true },
  { "NPO Radio 3"     , "http://icecast.omroep.nl/3fm-sb-mp3"             , 33, true },
  { "NPO Radio 4"     , "http://icecast.omroep.nl/radio4-sb-mp3"          , 33, true },
  { "NPO Radio 5"     , "http://icecast.omroep.nl/radio5-sb-mp3"          , 33, true },     // 11
  { "NPO Radio 6"     , "http://icecast.omroep.nl/radio6-sb-mp3"          , 33, true },
  { "Radio Gelderland", "http://stream.omroepgelderland.nl/radiogelderland/pls.php" , 33, true },
  { "Radio 10"         , "http://stream.radio10.nl/radio10"               , 33, true },
  //{ "Nieuws NOS"      , "http://www.internettuner.nl/nos/nieuws"          , 33, true },
  //{ "BNR Nieuws"      , "http://icecast-bnr.cdp.triple-it.nl/bnr_mp3_32_03" , 33, true },

  { "NPO Radio 1 ACC" , "http://icecast.omroep.nl/radio1-sb-aac"          , 33, false },    // 15
  { "NPO Radio 2 ACC" , "http://icecast.omroep.nl/radio2-sb-aac"          , 33, false },
  { "NPO Radio 3 ACC" , "http://icecast.omroep.nl/3fm-sb-aac"             , 33, false },
  { "NPO Radio 4 ACC" , "http://icecast.omroep.nl/radio4-sb-aac"          , 33, false },
  { "NPO Radio 5 ACC" , "http://icecast.omroep.nl/radio5-bb-aac"          , 20, false },    // 19
  { "NPO Radio 6 ACC" , "http://icecast.omroep.nl/radio6-sb-aac"          , 33, false },

  { "NashvilleFM"     , "http://nashvillefm.stream-server.nl:8300/stream" , 33, true },

  { "Asia Dream"      , "https://igor.torontocast.com:1025/;.mp3"         , 33, true },
  { "KPop Radio"      , "http://streamer.radio.co/s06b196587/listen"      , 33, true },
  { "Classic FM"      , "http://media-ice.musicradio.com:80/ClassicFMMP3" , 33, true },
  { "Lite Favorites"  , "http://naxos.cdnstream.com:80/1255_128"          , 33, true },
  { "MAXXED Out"      , "http://149.56.195.94:8015/steam"                 , 33, true },
  { "SomaFM Xmas"     , "http://ice2.somafm.com/christmas-128-mp3"        , 33, true },
  { 0, 0, 0}
};

const   int   stationListNumber   = sizeof(stationList) / sizeof(stationList[0]) - 1;
const   int   StationListDefault  = 11;
