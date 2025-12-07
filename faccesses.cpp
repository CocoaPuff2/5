#include <fcntl.h>    // open
#include <unistd.h>   // write, close
#include <iostream>   // cerr
#include <stdlib.h>   // atoi
#include <sys/time.h> // gettimeofday

#define NCHARS    26
#define FILEBLOCK 4096

using namespace std;

int filewrite( const char* filename, int nblocks, char* block ) {
  // open a file for writing.
  // WRONLY: opens file, write access only
  // CREAT: create file is not already exists
  // TRUNC: if file exists, truncate to 0 bytes, erases all content
  // 0666 is permission bits for writing a new file
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
      cerr << "Error opening file to write" << endl;
      return -1;
  }

  // start timer
  struct timeval start_time, end_time;
  // record current time before writing begins
  gettimeofday(&start_time, NULL);

  // repetitively write n times, (nblocks) of block (4096B)
  // writes nblocks that are 4096 each
  // fd: int, returned by open, uniquely identifies the open file in the OS.
  //     used in write(), read(), and close() to refer to that file.

  // block: pointer to a memory buffer (char*) of size FILEBLOCK that holds the data to
  //         write to the file (or where data read into)

  // FILEBLOCK:  macro defined as 4096, representing the size of one file block in bytes (4 KB)
  for (int i = 0; i < nblocks; i++) {
      // Attempt to write FILEBLOCK (4096) bytes from the buffer to the file
      ssize_t bytes_written = write(fd, block, FILEBLOCK);
      if (bytes_written < 0) {
          cerr << "Writing error." << endl;
          close(fd);
          return -1;
      } else if (bytes_written < FILEBLOCK) {
          cerr << "Partial write occurred." << endl;
          close(fd);
          return -1;
      }
  }

  close(fd); // flush all in-cache data to disk

  // stop timer
  gettimeofday( &end_time, NULL );

  time_t elapsed_time = ( end_time.tv_sec - start_time.tv_sec ) * 1000000 + ( end_time.tv_usec - start_time.tv_usec );
  cerr << "nblocks = " << nblocks << ", total time = " <<  elapsed_time << ", time / block = "<< elapsed_time / nblocks << endl;
  
  return 0;
}

int fileread( const char* filename, int nblocks, char* block ) {
  // open a file for reading.
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
      cerr << "Read error, can't open file " << endl;
      return -1;
  }

  // start timer
  struct timeval start_time, end_time;
  // get time before reading
  gettimeofday(&start_time, NULL);

  // repetitively read n times, (nblocks) of block (4096B)
  // reads nblocks of size 4096
  for (int i = 0; i < nblocks; i++) {
      ssize_t bytes_read = read(fd, block, FILEBLOCK);
      if (bytes_read < 0) {
          cerr << "Reading error" << endl;
          close(fd);
          return -1;
      } else if (bytes_read < FILEBLOCK) {
          cerr << "Partial read occured" << endl;
          close(fd);
          return -1;
      }
  }

  // stop timer
  gettimeofday( &end_time, NULL );

  time_t elapsed_time = ( end_time.tv_sec - start_time.tv_sec ) * 1000000 + ( end_time.tv_usec - start_time.tv_usec );
  cerr << "nblocks = " << nblocks << ", total time = " <<  elapsed_time << ", time / block = "<< elapsed_time / nblocks << endl;

  close(fd); // read closes after the timer stops as there are no data remained in cache.
  
  return 0;
}

// returns the name of a file that includes i blocks.
string filename( int i ){
  string num = to_string( i );
  string filename_str = "f_" + num + ".txt";
  return filename_str;
}

int main(int argc, char* argv[]) {
    // initialize block with repeating letters
    char block[FILEBLOCK];
    for (int i = 0; i < FILEBLOCK; i++)
        block[i] = 'a' + (i % NCHARS);

    cout << "writing to direct blocks ********************************************" << endl;
    filewrite(filename(1).c_str(), 1, block);    // 1 block, first direct pointer
    filewrite(filename(12).c_str(), 12, block);  // 12 blocks, all direct pointers

    cout << "writing to 1st indirect blocks **************************************" << endl;
    int single_indirect_blocks = 256; // 4 KB / 4 Bytes per pointer = 1024 ptrs, so went with 256
    filewrite(filename(13).c_str(), 13, block);              // direct + first of single indirect
    filewrite(filename(12 + single_indirect_blocks).c_str(), 12 + single_indirect_blocks, block); // full single indirect

    cout << "writing to 2ndary indirect blocks *******************************" << endl;
    int double_indirect_blocks = 256 * 256;
    filewrite(filename(12 + single_indirect_blocks + 1).c_str(), 12 + single_indirect_blocks + 1, block); // first block of double indirect
    filewrite(filename(12 + single_indirect_blocks + double_indirect_blocks).c_str(),
              12 + single_indirect_blocks + double_indirect_blocks, block); // full double indirect

    // --- Triple indirect blocks ---
    cout << "writing to 3tiary indirect blocks *******************************" << endl;
    int triple_indirect_blocks = 256 * 256 * 256;
    filewrite(filename(12 + single_indirect_blocks + double_indirect_blocks + 1).c_str(),
              12 + single_indirect_blocks + double_indirect_blocks + 1, block);

    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    // --- Reading files ---
    cout << "reading direct blocks ********************************************" << endl;
    fileread(filename(1).c_str(), 1, block);
    fileread(filename(12).c_str(), 12, block);

    cout << "reading 1st indirect blocks **************************************" << endl;
    fileread(filename(13).c_str(), 13, block);
    fileread(filename(12 + single_indirect_blocks).c_str(), 12 + single_indirect_blocks, block);

    cout << "reading 2ndary indirect blocks *******************************" << endl;
    fileread(filename(12 + single_indirect_blocks + 1).c_str(), 12 + single_indirect_blocks + 1, block);
    fileread(filename(12 + single_indirect_blocks + double_indirect_blocks).c_str(),
             12 + single_indirect_blocks + double_indirect_blocks, block);

    cout << "reading 3tiary indirect blocks *******************************" << endl;
    fileread(filename(12 + single_indirect_blocks + double_indirect_blocks + 1).c_str(),
             12 + single_indirect_blocks + double_indirect_blocks + 1, block);

    return 0;
}
