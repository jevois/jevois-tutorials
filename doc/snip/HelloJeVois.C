#include <jevois/Core/Module.H>
#include <jevois/Image/RawImageOps.H>

// icon by Catalin Fertu in cinema at flaticon

//! JeVois sample module
/*! This module is provided as an example of how to create a new standalone module.

    JeVois provides helper scripts and files to assist you in programming new modules, following two basic formats:

    - if you wish to only create a single module that will execute a specific function, or a collection of such modules
      where there is no shared code between the modules (i.e., each module does things that do not relate to the other
      modules), use the skeleton provided by this sample module. Here, all the code for the sample module is compiled
      into a single shared object (.so) file that is loaded by the JeVois engine when the corresponding video output
      format is selected by the host computer.

    - if you are planning to write a collection of modules with some shared algorithms among several of the modules, it
      is better to first create machine vision Components that implement the algorithms that are shared among several of
      your modules. You would then compile all your components into a first shared library (.so) file, and then compile
      each module into its own shared object (.so) file that depends on and automatically loads your shared library file
      when it is selected by the host computer. The jevoisbase library and collection of components and modules is an
      example for how to achieve that, where libjevoisbase.so contains code for Saliency, ObjectRecognition, etc
      components that are used in several modules, and each module's .so file contains only the code specific to that
      module.

    @author Sample Author

    @videomapping YUYV 640 480 28.5 YUYV 640 480 28.5 SampleVendor HelloJeVois
    @email sampleemail\@samplecompany.com
    @address 123 First Street, Los Angeles, CA 90012
    @copyright Copyright (C) 2017 by Sample Author
    @mainurl http://samplecompany.com
    @supporturl http://samplecompany.com/support
    @otherurl http://samplecompany.com/about
    @license GPL v3
    @distribution Unrestricted
    @restrictions None */
class HelloJeVois : public jevois::Module
{
  public:
    //! Default base class constructor ok
    using jevois::Module::Module;

    //! Virtual destructor for safe inheritance
    virtual ~HelloJeVois() { }

    //! Processing function
    virtual void process(jevois::InputFrame && inframe, jevois::OutputFrame && outframe) override
    {
      // Wait for next available camera image:
      jevois::RawImage const inimg = inframe.get(true);

      // We only support YUYV pixels in this example, any resolution:
      inimg.require("input", inimg.width, inimg.height, V4L2_PIX_FMT_YUYV);
      
      // Wait for an image from our gadget driver into which we will put our results:
      jevois::RawImage outimg = outframe.get();

      // Enforce that the input and output formats and image sizes match:
      outimg.require("output", inimg.width, inimg.height, inimg.fmt);
      
      // Just copy the pixel data over:
      memcpy(outimg.pixelsw<void>(), inimg.pixels<void>(), std::min(inimg.buf->length(), outimg.buf->length()));

      // Print a text message:
      jevois::rawimage::writeText(outimg, "Hello JeVois!", 100, 230, jevois::yuyv::White, jevois::rawimage::Font20x38);

      // Let camera know we are done processing the input image:
      inframe.done(); // NOTE: optional here, inframe destructor would call it anyway

      // Send the output image with our processing results to the host over USB:
      outframe.send(); // NOTE: optional here, outframe destructor would call it anyway
    }
};

// Allow the module to be loaded as a shared object (.so) file:
JEVOIS_REGISTER_MODULE(HelloJeVois);
