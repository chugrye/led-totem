using Newtonsoft.Json;
using System;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Reflection;
using System.Threading;

namespace syncstrip
{
    static class Program
    {
        private static int PixelsPerLine = 30;
        private static int NumLines = 5;
        private static int FramesPerSecond = 15;
        private static int NumColors = 3;
        private static int DelayVal = 1000 / FramesPerSecond;



        static void Main(string[] args)
        {
            ProgramConfig config = JsonConvert.DeserializeObject<ProgramConfig>(File.ReadAllText(Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "config.json")));

            // Initialize serial device
            using (SerialPort serial = new SerialPort(config.SerialPort, 1228800))
            {
                serial.DtrEnable = true; // Triggers a board reset, which prevents some strange junk bytes arriving to the chip
                serial.RtsEnable = true;
                serial.Open();
                Console.WriteLine($"Opened serial port");

                // Throw away any stale data
                serial.DiscardInBuffer();

                Console.WriteLine("Waiting for device to reset...");
                serial.ReadByte();
                Console.WriteLine("Device ready");

                byte[,,,] animation = RenderAnimation(BuildAnimation());
                //for (int frame = 0; frame < animation.GetLength(0); RenderAnimation())
                while (true)
                {
                    for (int frame = 0; frame < animation.GetLength(0); frame++)
                    {
                        SendFrame(serial, animation, frame);
                        Thread.Sleep(DelayVal);
                        // indicate the end of the render
                    }
                }
                //SendFrame(serial, animation, 1);
                //serial.Write(new byte[] { 0xFF }, 0, 1);
            }
        }

        static void SendFrame(SerialPort serial, byte[,,,] animation, int frameSlice)
        {

            byte[] frameByte = TransformToOneDimensionalArray(animation, frameSlice);
            Console.WriteLine(frameByte.Length);
            int j = 0;
            byte[] header = new byte[] { 0x01 };//, (byte)(test.Length/3)};
            byte[] footer = new byte[] { 0x00 };


            Console.WriteLine("writing data");
            serial.Write(header, 0, header.Length);
            serial.Write(frameByte, 0, frameByte.Length);

            serial.Write(footer, 0, footer.Length);
            serial.ReadByte();

        }

        private static byte[] TransformToOneDimensionalArray(byte[,,,] animation, int frameSlice)
        {
            byte[] frameByte = new byte[NumLines * PixelsPerLine * NumColors];
            int iterator = 0;
            for (int line = 0; line < animation.GetLength(1); line++)
            {
                for (int pixel = 0; pixel < animation.GetLength(2); pixel++)
                {
                    for (int color = 0; color < animation.GetLength(3); color++)
                    {
                        //                        Console.WriteLine(iterator);
                        frameByte[iterator] = animation[frameSlice, line, pixel, color];
                        iterator++;
                    }
                }
            }
            return frameByte;
        }

        private static byte[,,,] RenderAnimation(byte[,,] designPattern)
        {


            int designWidth = designPattern.GetLength(0);
            int designHeight = designPattern.GetLength(1);
            byte[,,,] animation = new byte[designHeight + PixelsPerLine, NumLines, PixelsPerLine, NumColors];
            for (int designPatternOffset = 0; designPatternOffset < designHeight + PixelsPerLine; designPatternOffset++)
            {
                for (int line = 0; line < NumLines; line++)
                {
                    for (int pixel = 0; pixel < PixelsPerLine; pixel++)
                    {
                        for (int color = 0; color < NumColors; color++)
                        {
                            if (designPatternOffset - pixel < 0 || designPatternOffset - pixel > designHeight - 1)
                            {

                                animation[designPatternOffset, line, pixel, color] = 0;
                            }
                            else
                            {
                                animation[designPatternOffset, line, pixel, color] = designPattern[line, designPatternOffset - pixel, color];
                            }
                        }
                    }
                }

            }
            return animation;
        }

        private static byte[,,] BuildAnimation()
        {
            byte[,,] designPattern = new byte[,,]
            {
                {
                    {120,120,0}, {120,120,0}, {120,120,0}, {120,120,0}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {120,0,120}, {120,0,120}
                },
                {
                    {120,120,0}, {120,120,0}, {120,120,0}, {120,120,0}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {120,0,120}, {120,0,120}
                },
                {
                    {120,120,0}, {120,120,0}, {120,120,0}, {120,120,0}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {120,0,120}, {120,0,120}
                },
                {
                    {120,120,0}, {120,120,0}, {120,120,0}, {120,120,0}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {120,0,120}, {120,0,120}
                },
                {
                    {120,120,0}, {120,120,0}, {120,120,0}, {120,120,0}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {0,120,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {0,0,0}, {120,0,120}, {120,0,120}, {120,0,120}
                }
            };
            byte[,,] designPattern2 = new byte[,,]
            {
                {
                    {120,120,0}, {0,0,0},{0,0,0}, {120,120,0}, {0,0,0}, {0,0,0}, {0,120,120}, {0,120,120}, {0,120,120},{0,120,120},  {0,0,0},{0,0,0}, {120,0,120},{0,0,0},{0,0,0}, {120,0,120},{0,0,0},{0,0,0}, {120,0,120}
                },
                {
                    {120,120,0}, {0,0,0},{0,0,0}, {120,120,0}, {0,0,0}, {0,0,0}, {0,120,120}, {0,0,0}, {0,0,0}, {0,120,120},  {0,0,0 },{0,0,0}, {0,0,0}, {0,0,0 },{0,0,0}, {0,0,0}, {0,0,0 },{0,0,0}, {0,0,0}
                },
                {
                    {120,120,0}, {120,120,0}, {120,120,0},{120,120,0}, {0,0,0}, {0,0,0}, {0,120,120}, {0,0,0},{0,0,0}, {0,120,120},  {0,0,0}, {0,0,0}, {120,0,120},  {0,0,0}, {0,0,0}, {120,0,120},  {0,0,0}, {0,0,0}, {120,0,120}
                },
                {
                    {120,120,0}, {0,0,0},{0,0,0}, {120,120,0}, {0,0,0}, {0,0,0}, {0,120,120}, {0,0,0},{0,0,0}, {0,120,120},  {0,0,0}, {0,0,0}, {120,0,120},  {0,0,0}, {0,0,0}, {120,0,120},  {0,0,0}, {0,0,0}, {120,0,120}
                },
                {
                    {120,120,0}, {0,0,0},{0,0,0}, {120,120,0}, {0,0,0}, {0,0,0}, {0,120,120}, {0,120,120}, {0,120,120}, {0,120,120},  {0,0,0},{0,0,0}, {120,0,120},  {0,0,0},{0,0,0}, {120,0,120},  {0,0,0},{0,0,0}, {120,0,120}
                }
            };
            return designPattern2;
        }
    }
}
