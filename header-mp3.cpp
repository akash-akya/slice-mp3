#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 4096
#define prettyPrint(a, b) (cout << " - " << (a) << "\t:   " << b << endl)
 
using namespace std;

class Time
{
  int minute;
  int second;

public:

  Time () {  }
    
  void setTime (long seconds)
  {
    minute = seconds / 60;
    second = seconds % 60;
  }

  void setTime (char arg[])
  {
    char *delimiterPos = strstr(arg,":");
    char minStr[10];
  
    second = atoi (delimiterPos+1);
    strncpy (minStr, arg, delimiterPos-arg);
    minStr[strlen(minStr)] = '\0';
    minute += atol(minStr);
  }

  long getTimeKiloBits()
  {
    return (minute * 60 + second) / 8.0 * 1000;
  }
    
  void setMinute (int min)  {    minute = min;  }
  
  void setSecond (int sec)  {    second = sec;  }
  
  int getMinute ()  { return minute;  }
  
  int getSecond ()  { return second;  }

  char* toString (char str[])
  {
    sprintf (str, "%dmin %dsec", minute, second);
    return str;
  } 

  bool isNotSet ()  { return (minute == 0 && second == 0); }
  
};

const int BitRateIndex[][6] = {
  {0,   0,   0,   0,   0},
  {32,  32,  32,  32,  8},
  {64,  48,  40,  48,  16},
  {96,  56,  48,  56,  24},
  {128, 64,  56,  64,  32},
  {160, 80,  64,  80,  40},
  {192, 96,  80,  96,  48},
  {224, 112, 96,  112, 56},
  {256, 128, 112, 128, 64},
  {288, 160, 128, 144, 80},
  {320, 192, 160, 160, 96},
  {352, 224, 192, 176, 112},
  {384, 256, 224, 192, 128},
  {416, 320, 256, 224, 144},
  {448, 384, 320, 256, 160},
  {-1,  -1,   -1,  -1,  -1}
};

const int SamplesPerFrame[][3] = {
  {384,   384,   384,},
  {1152,  1152,  1152,},
  {1152,  576,   576}
};

const int SamplingRate [] = {
  44100,    22050,    11025,
  48000,    24000,    12000,
  32000,    16000,    8000
};

typedef struct HeadStruct
{
  int mpegVersion;
  int layer;
  int crcProtection;
  int bitRate;
  int samplingRate;
  int padding;
  int privateBit;
  int channel;
  int modeExt;
  int copyRight;
  int original;
  int emphasis;
} HeaderStruct;


// inline void prettyPrint (const char name[], void  *parameter)
// {
//   cout << " - " << name << "\t:   " << parameter << endl;  
// }


// inline void prettyPrint (const char name[], const char parameter[])
// {
//   cout << " - " << name << "\t:   " << parameter << endl;  
// }


int getVal (char buf[], int start, int len)
{
  int i;
  int val = 0;
    
  for(i=start; i<(start+len); i++)
    {
      val <<= 1;
      val |= buf [i];
    }

  return val; 
}

int getVersion (char buf[])
{
  switch (getVal(buf, 11, 2))
    {
    case 0 : return 3;
    case 2 : return 2;
    case 3 : return 1;
    default : return -1;
    }
} 

int getLayer (char buf[])
{
  switch (getVal(buf, 13,2))
    {
    case 0 : return 0;
    case 1 : return 3;
    case 2 : return 2;
    case 3 : return 1;
    default : return -1; 
    }
}

int getBitrate (char buf[])
{
  int row = getVal(buf, 16, 4);
  int col = 0;

  if (getVersion(buf) == 1)
    {
      col = getLayer(buf)-1;
      // return BitRateIndex[row][col];
    }
  else if (getVersion(buf) == 2 || getVersion(buf) == 3)
    {
      col = 3;
      
      if (getLayer(buf)==2 || getLayer(buf) == 3)
	{
	  col++;
	}
    }
  else
    {
      cout << "Bitrate Error !" << endl;
      return 0;
    }
  
  return BitRateIndex[row][col];
}


HeaderStruct parseHeader (unsigned char *t)
{
    int i, j;
    char buf [32];
    HeaderStruct head;
    int h;

    h = *((int *)t);

    for (j=3; j>=0; j--)
    {
      for (i=0; i<8; i++)
	{
	  buf[31-(j*8)-i] = (h & 0x1);
	  h = h >> 1;
	}
    }
    
    head.mpegVersion = getVersion (buf);
    head.layer = getLayer (buf);
    head.crcProtection= getVal(buf, 15, 1);
    head.bitRate      = getBitrate(buf);
    head.samplingRate = SamplingRate[getVal(buf, 20, 2)*2 + getVersion(buf) -1];
    head.padding      = getVal (buf, 22, 1);  
    head.privateBit   = getVal (buf, 23, 1);  
    head.channel      = getVal (buf, 24, 2);
    head.modeExt      = getVal (buf, 26, 2);
    head.copyRight    = getVal (buf, 27, 1);
    head.original     = getVal (buf, 28, 1);
    head.emphasis     = getVal (buf, 29, 2);

    return head;
}

void displayInfo(char name[], HeaderStruct &head, float size, Time time)
{  	  
  char  t[15];

  cout << "\n -------------------------------------- \n";
  prettyPrint("File ", name);
  prettyPrint("Version", head.mpegVersion);
  prettyPrint("Layer", head.layer);
  prettyPrint("CRC code",((head.crcProtection) ? "False" : "True"));
  prettyPrint("Bitrate ", head.bitRate << " kbps");
  prettyPrint("SmaplingRate", head.samplingRate << " Hz");
  prettyPrint("Padding", (head.padding ? "Padded" : "Not Padded"));
  prettyPrint("Private Bit", head.privateBit);
  prettyPrint("Channel", head.channel);
  prettyPrint("Mode Ext", head.modeExt);  
  prettyPrint("Padding ", (head.copyRight ?
			   "Copyright protected" : "Not copyright protected"));
  prettyPrint("Original", (head.original ?
			   "Original Media" : "Not original Media"));
  prettyPrint("Emphasis", head.emphasis);
  prettyPrint("File Size", size << "MB");
  prettyPrint("Duration", time.toString(t));
}

void error_exit (const char msg[])
{
  cout << msg << endl;
  exit (1);
}

long getTagSize (FILE *file)
{
  int next = 0;
  int prev = 0;
  int cur = 0;
  long tagSize = 0;

  rewind (file);
  next = fgetc (file);
  while (next != EOF && (prev != 0xFF || ((cur&0xE0) != 0xE0)))
    {
      tagSize++;
      prev = cur;  cur = next;
      next = fgetc (file);
    }
  tagSize -= 2;
  return tagSize;
}
  

HeaderStruct getData (FILE *mp3File, long tagSize)
{
  unsigned char temp[4];
  HeaderStruct head;
  long frameSize = 0;
  float avgBitrate = 0;
  long frameCount = 0;
  long spf = 0;

  fseek (mp3File, tagSize, SEEK_SET);
  
   while (fread (&temp, 4, sizeof(char), mp3File))
    {
      if(temp[0] == 0xFF && (temp[1] & 0xE0) == 0xE0)
	{	  
	  head = parseHeader(temp);
	  avgBitrate = (avgBitrate*frameCount+head.bitRate)/(frameCount+1);
	  frameCount++; 

	  spf = SamplesPerFrame[head.layer-1][head.mpegVersion-1];
	  frameSize = ((spf / 8 * head.bitRate * 1000) / head.samplingRate)
	    + head.padding;
	  frameSize += (head.crcProtection > 0) ? 0 : 2; 
	  frameSize -= 4;
	}
      else
	{
	  break;
	}
      fseek(mp3File, frameSize, SEEK_CUR);
    }
   head.bitRate = avgBitrate;
   return head;
}

long timeSeek (FILE *file, long offset, long duration)
{
  unsigned char temp[4];
  HeaderStruct head;
  long curDur      = 0;
  long frameSize   = 0;
  int  spf         = 0;
  int  frameCount  = 0;
  float avgBitrate = 0;

  fseek (file, offset, SEEK_SET);
  
  while (curDur <= duration)
    {
      fread  (&temp, 4, sizeof(char), file);   
      head = parseHeader(temp);
	  
      spf = SamplesPerFrame[head.layer-1][head.mpegVersion-1];
      frameSize = ((spf / 8 * head.bitRate * 1000) / head.samplingRate)
	+ head.padding;
      frameSize += (head.crcProtection > 0) ? 0 : 2;
      fseek (file, frameSize-4, SEEK_CUR);     
      avgBitrate = (avgBitrate*(frameCount)+head.bitRate)/(frameCount+1);
      curDur = ((ftell(file)-offset) / avgBitrate); 
      frameCount++;
    }
  return ftell(file)-frameSize;
}

void optionsInfo (char sourceFile[], char outputFile[],
		  Time sourceTime,
		  Time startTime,
		  Time endTime,
		  Time copyTime,
		  Time leaveTime)
{
  char time [15];
  
  prettyPrint ("SOURCE", sourceFile);
  prettyPrint ("OUTPUT", outputFile);
  prettyPrint ("SOURCE TIME", sourceTime.toString(time));
  prettyPrint ("START TIME", startTime.toString(time));
  prettyPrint ("END TIME", endTime.toString(time));
  prettyPrint ("COPY TIME", copyTime.toString(time));
  prettyPrint ("LEAVE TIME", leaveTime.toString(time));
}

long getFileSize (FILE *file)
{
  fseek(file, 0L, SEEK_END);
  return ftell(file) / 1000000.0;
}

int main (int argc, char *argv[])
{
  HeaderStruct  head; 
  FILE *mp3File   = NULL;
  FILE *newFile;
  float duration;
  float avgBitrate = 0;
  char *arg;
  char temp[BUF_SIZE];
  char  sourceFile[20] = "";
  char  outputFile[20] = "";
  int   fileSize;
  int   endPos;
  int   tagSize = 0;
  Time  sourceTime;
  Time  endTime;
  Time  startTime;
  Time  copyTime;
  Time  leaveTime;
  int times = 0;
  int reminder = 0;
  long curTimeDuration = 0;
  long curPosition = 0;
  long len = 0;      
  

  if (argc <= 1)
    {
      error_exit ("Invalid input!");
    }
  
  // Parse input
  int i=1;
  while ( i<argc)
    {
      arg = argv[i++];
      
      if (strcmp(arg, "-s") == 0)
	{
	  if (i<argc)
	    {
	      strcpy (sourceFile, argv[i++]);
	    }
	  else
	    {
	      error_exit ("Source file name is not provided");
	    }
	}
      else if (strcmp(arg, "-o") == 0)
	{
	  if (i<argc)
	    {
	      strcpy (outputFile, argv[i++]);
	    }
	  else
	    {
	      error_exit ("Ouput file name is not provided");
	    }
	}      
      else if (strcmp(arg, "-et") == 0)
	{
	  if (i<argc)
	    {
	      // long time = parseTime(argv[i]);
	      // outMin = time / 60;
	      // outSec = time % 60;
	      endTime.setTime(argv[i]);
	      i++;	      
	    }
	  else
	    {
	      error_exit ("End time is not provided not provided");
	    }
	}
      else if (strcmp(arg, "-st") == 0)
	{
	  if (i<argc)
	    {	    
	      startTime.setTime (argv[i]);
	      i++;
	      
	    }
	  else
	    {
	      error_exit ("Start time is not provided not provided");
	    }
	}
      else if (strcmp(arg, "-ct") == 0)
	{
	  if (i<argc)
	    {
	      copyTime.setTime (argv[i]);
	    }
	  else
	    {
	      error_exit ("Start time is not provided not provided");
	    }
	}
      else if (strcmp(arg, "-lt") == 0)
	{
	  if (i<argc)
	    {      

	      leaveTime.setTime (argv[i]);
	      i++;      
	    }
	  else
	    {
	      error_exit ("Start time is not provided not provided");
	    }
	}

    }


  if (!strcmp(sourceFile, "") || !strcmp(outputFile, ""))
    {
      error_exit("Invalid arguments"); 
    }  
  
  
  if(!(mp3File = fopen (sourceFile, "rb")))
    {
      error_exit("Can not open file!!");
    }

  if(!(newFile = fopen (outputFile, "wb+")))
    {
      error_exit("Invalid file name");
    } 

  optionsInfo(sourceFile, outputFile,
	      sourceTime, startTime,
	      endTime, copyTime, leaveTime);
  
  tagSize = getTagSize(mp3File);

  // cout << "ID Tag Size  : " << tagSize << " Bytes" << endl;
      
  fseek(mp3File, tagSize, SEEK_SET);
  head = getData (mp3File, tagSize);
  avgBitrate = head.bitRate;  
  endPos = ftell(mp3File);

  duration  = ((endPos-tagSize) / avgBitrate * 8.0 / 1000) ;
  //min = duration / 60;
  //sec = duration-min*60;

  sourceTime.setTime (duration);

  fileSize = getFileSize(mp3File);
  displayInfo (sourceFile, head, fileSize, sourceTime);  

  rewind (mp3File);
  
  fread (&temp, tagSize, sizeof(char), mp3File);
  fwrite(&temp, tagSize, sizeof(char), newFile);

  if (endTime.isNotSet())
    {
      endTime.setMinute (sourceTime.getMinute());
      endTime.setSecond (sourceTime.getSecond());
    }

  //endTimeDuration  = (outMin   * 60 + outSec)   / 8.0 * 1000;  
  //copyTimeDuration = (copyTimeDuration) ? copyTimeDuration : endTimeDuration;
  
  // curTimeDuration = startTimeDuration;
  curTimeDuration = startTime.getTimeKiloBits();
  
  while (curTimeDuration+copyTime.getTimeKiloBits() <= endTime.getTimeKiloBits())
    {
      curPosition = timeSeek(mp3File, tagSize, curTimeDuration);
      curTimeDuration += copyTime.getTimeKiloBits();

      len = timeSeek(mp3File, tagSize, curTimeDuration) - curPosition;
		          
      times = len / BUF_SIZE;
      reminder = len % BUF_SIZE;

      fseek (mp3File, curPosition, SEEK_SET);
      for (int i = 0 ; i < times ; i++)
	{      
	  fread  (temp, BUF_SIZE, 1, mp3File);
	  fwrite (temp, BUF_SIZE, 1, newFile);
	}  
      fread  (temp, reminder, 1, mp3File);
      fwrite (temp, reminder, 1, newFile);

      curTimeDuration += leaveTime.getTimeKiloBits(); 
    }


  head = getData (newFile, tagSize);  
  avgBitrate = head.bitRate;

  endPos = ftell(newFile);
  duration  = ((endPos-tagSize) / avgBitrate * 8.0 / 1000) ;

  // min = duration / 60;
  // sec = duration-min*60;
  endTime.setTime (duration);

  fileSize = getFileSize(newFile);
  displayInfo (outputFile, head, fileSize, endTime);  

  fclose(newFile);
  fclose(mp3File);

  return 0;
}
