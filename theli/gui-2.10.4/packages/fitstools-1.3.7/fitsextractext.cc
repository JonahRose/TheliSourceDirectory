#include <CCfits>
#include <fitsio.h>
#include "fitstools.h"
#include "statistics.h"

using namespace std;

void usage(int i, char *argv[])
{
  if (i == 0) {
    cerr << "\n";
    cerr << "  USAGE: " << argv[0] << endl;
    cerr << "           -i input_image\n";
    cerr << "           -o output_image\n";
    cerr << "           -e extension\n";
    cerr << "          [-n (print the number of extensions in the file)]\n";
    cerr << "      Extracts the specified extension form a MEF image.\n\n";
    cerr << "      If -n is given, only the number of extensions is printed.\n\n";
    exit(1);
  }
}


//************************************************************
int main(int argc, char *argv[])
{
  bool do_next = false;
  long i, extension = 0;
  long nax1, nax2, n_ext;
  string input_image, output_image;

  // print usage if no arguments were given
  if (argc==1) usage(0, argv);

  for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      switch(tolower((int)argv[i][1])) {
        case 'i': input_image = argv[++i];
            break;
        case 'o': output_image = argv[++i];
            break;
        case 'e': extension = atoi(argv[++i]);
            break;
        case 'n': do_next = true;
            break;
      }
    }
  }

  checkfile(input_image);
  string call = programcall(argv, argc);

  // read the FITS header and the data block
  myFITS image(input_image);
  n_ext = image.numExt();

  if (do_next) {
    myFITS image(input_image, false);
    n_ext = image.numExt();
    cout << n_ext << endl;
    return 0;
  }
  else {
    myFITS image(input_image);
    n_ext = image.numExt();
  }

  if (n_ext < extension) {
    cerr << "ERROR: Extension " << extension << " does not exist in " << input_image << endl;
    cerr << "Only " << n_ext << " extensions were found.\n";
    return 1;
  }

  // read the data block
  vector<float> outdata;
  image.readExtData(outdata, extension);

  nax1 = image.readLongExtKey("NAXIS1", extension);
  nax2 = image.readLongExtKey("NAXIS2", extension);

  writeImage(output_image, outdata, nax1, nax2, -32);

  return 0;
}