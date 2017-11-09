// A Hello World! program in C#.
using System;
namespace HelloWorld
{
    class Hello 
    {
        static void Main() 
        {
            #region simulate errors by uncommenting these things (which is the purpose of this cs file)
            char dcs = Path.DirectorySeparatorChar;
            string uhoh = "uhoh";
            #endregion

            Console.WriteLine("Hello World!");

            // Keep the console window open in debug mode.
            Console.WriteLine("Press any key to exit.");
            Console.ReadKey();
        }
    }
}
