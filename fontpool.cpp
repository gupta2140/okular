// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
//
// fontpool.cpp
//
// (C) 2001-2005 Stefan Kebekus
// Distributed under the GPL

#include <config.h>

#include "fontpool.h"
#include "kvs_debug.h"
#include "TeXFont.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kprocio.h>

#include <qapplication.h>

#include <cmath>

//#define DEBUG_FONTPOOL


// List of permissible MetaFontModes which are supported by kdvi.

//const char *MFModes[]       = { "cx", "ljfour", "lexmarks" };
//const char *MFModenames[]   = { "Canon CX", "LaserJet 4", "Lexmark S" };
//const int   MFResolutions[] = { 300, 600, 1200 };

#ifdef PERFORMANCE_MEASUREMENT
QTime fontPoolTimer;
bool fontPoolTimerFlag;
#endif

fontPool::fontPool()
  :  progress("fontgen",  // Chapter in the documentation for help.
              i18n("KDVI is currently generating bitmap fonts..."),
              i18n("Aborts the font generation. Don't do this."),
              i18n("KDVI is currently generating bitmap fonts which are needed to display your document. "
                   "For this, KDVI uses a number of external programs, such as MetaFont. You can find "
                   "the output of these programs later in the document info dialog."),
              i18n("KDVI is generating fonts. Please wait."),
              0)
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::fontPool() called" << endl;
#endif

  setName("Font Pool");

  displayResolution_in_dpi = 100.0; // A not-too-bad-default
  useFontHints             = true;
  CMperDVIunit             = 0;
  extraSearchPath          = QString::null;
  fontList.setAutoDelete(true);

#ifdef HAVE_FREETYPE
  // Initialize the Freetype Library
  if ( FT_Init_FreeType( &FreeType_library ) != 0 ) {
    kdError(kvs::dvi) << "Cannot load the FreeType library. KDVI proceeds without FreeType support." << endl;
    FreeType_could_be_loaded = false;
  } else
    FreeType_could_be_loaded = true;
#endif
}


fontPool::~fontPool()
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::~fontPool() called" << endl;
#endif

#ifdef HAVE_FREETYPE
  if (FreeType_could_be_loaded == true)
    FT_Done_FreeType( FreeType_library );
#endif
}


void fontPool::setParameters( bool _useFontHints )
{
  // Check if glyphs need to be cleared
  if (_useFontHints != useFontHints) {
    double displayResolution = displayResolution_in_dpi;
    TeXFontDefinition *fontp = fontList.first();
    while(fontp != 0 ) {
      fontp->setDisplayResolution(displayResolution * fontp->enlargement);
      fontp=fontList.next();
    }
  }

  useFontHints = _useFontHints;
}


TeXFontDefinition* fontPool::appendx(const QString& fontname, Q_UINT32 checksum, Q_UINT32 scale, double enlargement)
{
  // Reuse font if possible: check if a font with that name and
  // natural resolution is already in the fontpool, and use that, if
  // possible.
  TeXFontDefinition *fontp = fontList.first();
  while( fontp != 0 ) {
    if ((fontname == fontp->fontname) && ( (int)(enlargement*1000.0+0.5)) == (int)(fontp->enlargement*1000.0+0.5)) {
      // if font is already in the list
      fontp->mark_as_used();
      return fontp;
    }
    fontp=fontList.next();
  }

  // If font doesn't exist yet, we have to generate a new font.

  double displayResolution = displayResolution_in_dpi;

  fontp = new TeXFontDefinition(fontname, displayResolution*enlargement, checksum, scale, this, enlargement);
  if (fontp == 0) {
    kdError(kvs::dvi) << i18n("Could not allocate memory for a font structure!") << endl;
    exit(0);
  }
  fontList.append(fontp);

#ifdef PERFORMANCE_MEASUREMENT
  fontPoolTimer.start();
  fontPoolTimerFlag = false;
#endif

  // Now start kpsewhich/MetaFont, etc. if necessary
  return fontp;
}


QString fontPool::status()
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::status() called" << endl;
#endif

  QString       text;
  QStringList   tmp;

  if (fontList.isEmpty())
    return i18n("The fontlist is currently empty.");

  text.append("<table WIDTH=\"100%\" NOSAVE >");
  text.append( QString("<tr><td><b>%1</b></td> <td><b>%2</b></td> <td><b>%3</b></td> <td><b>%4</b> <td><b>%5</b></td> <td><b>%6</b></td></tr>")
               .arg(i18n("TeX Name"))
               .arg(i18n("Family"))
               .arg(i18n("Zoom"))
               .arg(i18n("Type"))
               .arg(i18n("Encoding"))
               .arg(i18n("Comment")) );

 TeXFontDefinition *fontp = fontList.first();
  while ( fontp != 0 ) {
    QString errMsg, encoding;

    if (!(fontp->flags & TeXFontDefinition::FONT_VIRTUAL)) {
#ifdef HAVE_FREETYPE
      encoding = fontp->getFullEncodingName();
#endif
      if (fontp->font != 0)
        errMsg = fontp->font->errorMessage;
      else
        errMsg = i18n("Font file not found");
    }

#ifdef HAVE_FREETYPE
    tmp << QString ("<tr><td>%1</td> <td>%2</td> <td>%3%</td> <td>%4</td> <td>%5</td> <td>%6</td></tr>")
      .arg(fontp->fontname)
      .arg(fontp->getFullFontName())
      .arg((int)(fontp->enlargement*100 + 0.5))
      .arg(fontp->getFontTypeName())
      .arg(encoding)
      .arg(errMsg);
#endif

    fontp=fontList.next();
  }

  tmp.sort();
  text.append(tmp.join("\n"));
  text.append("</table>");

  return text;
}


bool fontPool::areFontsLocated()
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::areFontsLocated() called" << endl;
#endif

  // Is there a font whose name we did not try to find out yet?
  TeXFontDefinition *fontp = fontList.first();
  while( fontp != 0 ) {
    if ( !fontp->isLocated() )
      return false;
    fontp=fontList.next();
  }

#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "... yes, all fonts are located (but not necessarily loaded)." << endl;
#endif
  return true; // That says that all fonts are located.
}


void fontPool::locateFonts()
{
  kpsewhichOutput = QString::null;

  // First, we try and find those fonts which exist on disk
  // already. If virtual fonts are found, they will add new fonts to
  // the list of fonts whose font files need to be located, so that we
  // repeat the lookup.
  bool vffound;
  do {
    vffound = false;
    locateFonts(false, false, &vffound);
  } while(vffound);

  // If still not all fonts are found, look again, this time with
  // on-demand generation of PK fonts enabled.
  if (!areFontsLocated())
    locateFonts(true, false);

  // If still not all fonts are found, we look for TFM files as a last
  // resort, so that we can at least draw filled rectangles for
  // characters.
  if (!areFontsLocated())
    locateFonts(false, true);

  // If still not all fonts are found, we give up. We mark all fonts
  // as 'located', so that we won't look for them any more, and
  // present an error message to the user.
  if (!areFontsLocated()) {
    markFontsAsLocated();
    QString details = QString("<qt><p><b>PATH:</b> %1</p>%2</qt>").arg(getenv("PATH")).arg(kpsewhichOutput);
    KMessageBox::detailedError( 0, i18n("<qt><p>KDVI was not able to locate all the font files "
                                        "which are necessary to display the current DVI file. "
                                        "Your document might be unreadable.</p></qt>"),
                                details,
                                i18n("Not All Font Files Found") );
  }
}


void fontPool::locateFonts(bool makePK, bool locateTFMonly, bool *virtualFontsFound)
{
  // Set up the kpsewhich process. If pass == 0, look for vf-fonts and
  // disable automatic font generation as vf-fonts can't be
  // generated. If pass == 0, ennable font generation, if it was
  // enabled globally.
  emit setStatusBarText(i18n("Locating fonts..."));

  KProcIO kpsewhich_;

  // If PK fonts are generated, the kpsewhich command will re-route
  // the output of MetaFont into its stderr. Here we make sure this
  // output is intercepted and parsed.
  qApp->connect(&kpsewhich_, SIGNAL(receivedStderr(KProcess *, char *, int)),
                this, SLOT(mf_output_receiver(KProcess *, char *, int)));

  // Now generate the command line for the kpsewhich
  // program. Unfortunately, this can be rather long and involved...
  QStringList kpsewhich_args;
  kpsewhich_args << "kpsewhich"
                 << "--dpi" << "1200"
                 << "--mode" << "lexmarks";

  // Disable automatic pk-font generation.
  if (makePK == true)
    kpsewhich_args << "--mktex" << "pk";
  else
    kpsewhich_args << "--no-mktex" << "pk";

  // Names of fonts that shall be located
  Q_UINT16 numFontsInJob = 0;
  TeXFontDefinition *fontp = fontList.first();
  while ( fontp != 0 ) {
    if (!fontp->isLocated()) {
      numFontsInJob++;

      if (locateTFMonly == true)
        kpsewhich_args << QString("%1.tfm").arg(fontp->fontname);
      else {
#ifdef HAVE_FREETYPE
        if (FreeType_could_be_loaded == true) {
          const QString &filename = fontsByTeXName.findFileName(fontp->fontname);
          if (!filename.isEmpty())
            kpsewhich_args << QString("%1").arg(filename);
        }
#endif
        kpsewhich_args << QString("%1.vf").arg(fontp->fontname)
                       << QString("%1.1200pk").arg(fontp->fontname);
      }
    }
    fontp=fontList.next();
  }

  if (numFontsInJob == 0)
    return;

  progress.setTotalSteps(numFontsInJob, &kpsewhich_);

  // Now run... kpsewhich. In case of error, kick up a fuss.
  // This string is not going to be quoted, as it might be were it
  // a real command line, but who cares?
  kpsewhichOutput += "<p><b>" + kpsewhich_args.join(" ") + "</b></p>";

  const QString importanceOfKPSEWHICH =
    i18n("<p>KDVI relies on the <b>kpsewhich</b> program to locate font files "
         "on your hard disc and to generate PK fonts, if necessary.</p>");

  kpsewhich_ << kpsewhich_args;
  if (kpsewhich_.start(KProcess::NotifyOnExit, false) == false) {
    const QString msg =
      i18n("<p>There were problems running <b>kpsewhich</b>. As a result, "
           "some font files could not be located, and your document might be unreadable.</p>"
           "<p><b>Possible reason:</b> The kpsewhich program is perhaps not installed on your system, or it "
           "cannot be found in the current search path.</p>"
           "<p><b>What you can do:</b> The kpsewhich program is normally contained in distributions of the TeX "
           "typesetting system. If TeX is not installed on your system, you could install the TeTeX distribution (www.tetex.org). "
           "If you are sure that TeX is installed, please try to use the kpsewhich program from the command line to check if it "
           "really works.</p>");
    const QString details =
      QString("<qt><p><b>PATH:</b> %1</p>%2</qt>").arg(getenv("PATH")).arg(kpsewhichOutput);

    KMessageBox::detailedError(0,
                               QString("<qt>%1%2</qt>").arg(importanceOfKPSEWHICH).arg(msg),
                               details,
                               i18n("Problem locating fonts - KDVI"));

    // This makes sure the we don't try to run kpsewhich again
    markFontsAsLocated();
    return;
  }

  // We wait here while the kpsewhich program is concurrently
  // running. Every second we call processEvents() to keep the GUI
  // updated. This is important, e.g. for the progress dialog that is
  // shown when PK fonts are generated by MetaFont.
  while(kpsewhich_.wait(1) == false)
    qApp->processEvents();
  progress.hide();

  // Handle fatal errors.
  if (!kpsewhich_.normalExit()) {
    KMessageBox::sorry( 0, "<qt><p>The font generation was aborted. As a result, "
                        "some font files could not be located, and your document might be unreadable.</p></qt>",
                        i18n("Font generation aborted - KDVI") );

    // This makes sure the we don't try to run kpsewhich again
    if (makePK == false)
      markFontsAsLocated();
  }

  // Create a list with all filenames found by the kpsewhich program.
  QStringList fileNameList;
  QString line;
  while(kpsewhich_.readln(line) >= 0)
    fileNameList += line;

  // Now associate the file names found with the fonts
  fontp=fontList.first();
  while ( fontp != 0 ) {
    if (fontp->filename.isEmpty() == true) {
      QStringList matchingFiles;
#ifdef HAVE_FREETYPE
      const QString &fn = fontsByTeXName.findFileName(fontp->fontname);
      if (!fn.isEmpty())
        matchingFiles = fileNameList.grep(fn);
#endif
      if (matchingFiles.isEmpty() == true)
        matchingFiles += fileNameList.grep(fontp->fontname+".");

      if (matchingFiles.isEmpty() != true) {
#ifdef DEBUG_FONTPOOL
        kdDebug(kvs::dvi) << "Associated " << fontp->fontname << " to " << matchingFiles.first() << endl;
#endif
        QString fname = matchingFiles.first();
        fontp->fontNameReceiver(fname);
        fontp->flags |= TeXFontDefinition::FONT_KPSE_NAME;
        if (fname.endsWith(".vf")) {
          if (virtualFontsFound != 0)
            *virtualFontsFound = true;
          // Constructing a virtual font will most likely insert other
          // fonts into the fontList. After that, fontList.next() will
          // no longer work. It is therefore safer to start over.
          fontp=fontList.first();
          continue;
        }
      }
    } // of if (fontp->filename.isEmpty() == true)
    fontp = fontList.next();
  }
}


void fontPool::setCMperDVIunit( double _CMperDVI )
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::setCMperDVIunit( " << _CMperDVI << " )" << endl;
#endif

  if (CMperDVIunit == _CMperDVI)
    return;

  CMperDVIunit = _CMperDVI;

  TeXFontDefinition *fontp = fontList.first();
  while(fontp != 0 ) {
    fontp->setDisplayResolution(displayResolution_in_dpi * fontp->enlargement);
    fontp=fontList.next();
  }
}


void fontPool::setDisplayResolution( double _displayResolution_in_dpi )
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::setDisplayResolution( displayResolution_in_dpi=" << _displayResolution_in_dpi << " ) called" << endl;
#endif

  // Ignore minute changes by less than 2 DPI. The difference would
  // hardly be visible anyway. That saves a lot of re-painting,
  // e.g. when the user resizes the window, and a flickery mouse
  // changes the window size by 1 pixel all the time.
  if ( fabs(displayResolution_in_dpi - _displayResolution_in_dpi) <= 2.0 ) {
#ifdef DEBUG_FONTPOOL
    kdDebug(kvs::dvi) << "fontPool::setDisplayResolution(...): resolution wasn't changed. Aborting." << endl;
#endif
    return;
  }

  displayResolution_in_dpi = _displayResolution_in_dpi;
  double displayResolution = displayResolution_in_dpi;

  TeXFontDefinition *fontp = fontList.first();
  while(fontp != 0 ) {
    fontp->setDisplayResolution(displayResolution * fontp->enlargement);
    fontp=fontList.next();
  }

  // Do something that causes re-rendering of the dvi-window
  /*@@@@
  emit fonts_have_been_loaded(this);
  */
}


void fontPool::markFontsAsLocated()
{
  TeXFontDefinition *fontp=fontList.first();
  while ( fontp != 0 ) {
    fontp->markAsLocated();
    fontp = fontList.next();
  }
}



void fontPool::mark_fonts_as_unused()
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "fontPool::mark_fonts_as_unused() called" << endl;
#endif

  TeXFontDefinition  *fontp = fontList.first();
  while ( fontp != 0 ) {
    fontp->flags &= ~TeXFontDefinition::FONT_IN_USE;
    fontp=fontList.next();
  }
}


void fontPool::release_fonts()
{
#ifdef DEBUG_FONTPOOL
  kdDebug(kvs::dvi) << "Release_fonts" << endl;
#endif

  TeXFontDefinition  *fontp = fontList.first();
  while(fontp != 0) {
    if ((fontp->flags & TeXFontDefinition::FONT_IN_USE) != TeXFontDefinition::FONT_IN_USE) {
      fontList.removeRef(fontp);
      fontp = fontList.first();
    } else
      fontp = fontList.next();
  }
}


void fontPool::mf_output_receiver(KProcess *, char *buffer, int buflen)
{
  // Paranoia.
  if (buflen < 0)
    return;

  QString op = QString::fromLocal8Bit(buffer, buflen);

  kpsewhichOutput.append(op);
  MetafontOutput.append(op);

  // We'd like to print only full lines of text.
  int numleft;
  bool show_prog = false;
  while( (numleft = MetafontOutput.find('\n')) != -1) {
    QString line = MetafontOutput.left(numleft+1);
#ifdef DEBUG_FONTPOOL
    kdDebug(kvs::dvi) << "MF OUTPUT RECEIVED: " << line;
#endif
    // Search for a line which marks the beginning of a MetaFont run
    // and show the progress dialog at the end of this method.
    if (line.find("kpathsea:") == 0)
      show_prog = true;

    // If the Output of the kpsewhich program contains a line starting
    // with "kpathsea:", this means that a new MetaFont-run has been
    // started. We filter these lines out and update the display
    // accordingly.
    int startlineindex = line.find("kpathsea:");
    if (startlineindex != -1) {
      int endstartline  = line.find("\n",startlineindex);
      QString startLine = line.mid(startlineindex,endstartline-startlineindex);

      // The last word in the startline is the name of the font which we
      // are generating. The second-to-last word is the resolution in
      // dots per inch. Display this info in the text label below the
      // progress bar.
      int lastblank     = startLine.findRev(' ');
      QString fontName  = startLine.mid(lastblank+1);
      int secondblank   = startLine.findRev(' ',lastblank-1);
      QString dpi       = startLine.mid(secondblank+1,lastblank-secondblank-1);

      progress.show();
      progress.increaseNumSteps( i18n("Currently generating %1 at %2 dpi").arg(fontName).arg(dpi) );
    }
    MetafontOutput = MetafontOutput.remove(0,numleft+1);
  }
}

#include "fontpool.moc"
