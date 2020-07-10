using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.IO;
using ZedGraph;
using System.Threading.Tasks; 
namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {   /*      INITIAL VARIABLES       */
        int i = 0;  
        int byte_number;
        // STX = "1" , ETX = "9"
        byte[] data_in = new byte [13] {49, 44, 0 , 44 , 0 , 44, 0 ,44 ,0, 44, 0, 44, 57};
        byte[] data_out;
        int setpoint;
        byte[] temp = new byte[4];
        int data = 0;         
        int pre_data = 0;
        int pos;
        float w;
        byte[] test = new byte[4] { 0, 0, 0, 100 };
        float t;
        int ppr = 440;
        int TickStart, intMode = 1;
        int a = 0;
        int b = 0;
        byte pre_encoder_mode = 48;
        double time;
       
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();
            cbComPort.Items.AddRange(ports);
            data_out = new byte[11] { 49, 44, 48, 44, 48, 44, 48, 44, 48, 44, 57 };
            graph_initiate();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void cbBaudRate_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            try
            {   /*      GET COM PORT & BAUD RATE        */
                serialPort1.PortName = cbComPort.Text;
                serialPort1.BaudRate = Convert.ToInt32(cbBaudRate.Text);
                serialPort1.Open();
            }
            catch (Exception err)
            {
                MessageBox.Show(err.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                serialPort1.Close();
            }
        }

        private void textBox1_TextChanged_1(object sender, EventArgs e)
        {

        }

        private void label6_Click(object sender, EventArgs e)
        {

        }

        private void label7_Click(object sender, EventArgs e)
        {

        }

        private void tbIN_TextChanged(object sender, EventArgs e)
        {

        }


        private void cbComPort_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void tbWirte_TextChanged(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged_2(object sender, EventArgs e)
        {

        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (a == 0)                                 //  drawing graph only when first command is sent.
            {
                TickStart = Environment.TickCount;
                a = 1;
            }
            
            if (serialPort1.IsOpen)
            {
                if (checkBox2.Checked == true)
                    data_out[2] = 49;
                if (checkBox1.Checked == true)
                    data_out[2] = 48;
                if (checkBox3.Checked == true)
                {
                    data_out[4] = 48;
                }
                if (checkBox4.Checked == true)
                {
                    data_out[4] = 49;
                }
                if (data_out[4] != pre_encoder_mode)        //  reset data when encoder mode changed.
                {
                    pre_data = 0;
                    data = 0;
                }
                pre_encoder_mode = data_out[4];
                if (txSP.Text != "")
                    setpoint = int.Parse(txSP.Text);
                if (data_out[2] == 48)
                {
                    if (setpoint < 20)
                        setpoint = 20;
                    if (setpoint > 350)
                        setpoint = 350;
                }
                else if (data_out[2] == 49)
                {
                    if (setpoint < 50)
                        setpoint = 50;
                    if (setpoint > 220)
                        setpoint = 220;
                }
                data_out[6] = (byte)(setpoint / 256);
                data_out[8] = (byte)(setpoint - (setpoint / 256) * 256);
                serialPort1.Write(data_out, 0, 11);
                txSP.Text = setpoint.ToString();
                TimerOut.Enabled = true;
            }
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked == true)
                checkBox2.Enabled = false;
            else if (checkBox1.Checked == false)
                checkBox2.Enabled = true;
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox3.Checked == true)
                checkBox4.Enabled = false;
            else if (checkBox3.Checked == false)
                checkBox4.Enabled = true;
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked == true)
                checkBox1.Enabled = false;
            else if (checkBox2.Checked == false)
                checkBox1.Enabled = true;

        }

        private void checkBox4_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox4.Checked == true)
                checkBox3.Enabled = false;
            else if (checkBox4.Checked == false)
                checkBox3.Enabled = true;
        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void button1_Click_1(object sender, EventArgs e)
        {

        }



        private void zedGraphControl1_Load(object sender, EventArgs e)
        {

        }


        private void graph_initiate()
        {
            GraphPane myPane = zedGraphControl1.GraphPane;
            myPane.Title.Text = "graph";
            myPane.XAxis.Title.Text = "Thời gian (s)";
            myPane.YAxis.Title.Text = "data";
            RollingPointPairList list = new RollingPointPairList(60000);
            RollingPointPairList list1 = new RollingPointPairList(60000);
            LineItem curve = myPane.AddCurve("data", list, Color.Red, SymbolType.None);
            LineItem curve1 = myPane.AddCurve("set_point", list1, Color.Blue, SymbolType.None);
            myPane.XAxis.Scale.Min = 0;
            myPane.XAxis.Scale.Max = 30;
            myPane.XAxis.Scale.MinorStep = 1;
            myPane.XAxis.Scale.MajorStep = 5;
            zedGraphControl1.AxisChange();

        }
        private void draw(string setpoint, string current)
        {
            double intsetpoint;
            double intcurrent;
            double.TryParse(setpoint, out intsetpoint);
            double.TryParse(current, out intcurrent);
            if (zedGraphControl1.GraphPane.CurveList.Count <= 0)
                return;
            LineItem curve = zedGraphControl1.GraphPane.CurveList[0] as LineItem;
            LineItem curve1 = zedGraphControl1.GraphPane.CurveList[1] as LineItem;
            if (curve == null)
                return;
            if (curve1 == null)
                return;
            IPointListEdit list = curve.Points as IPointListEdit;
            IPointListEdit list1 = curve1.Points as IPointListEdit;
            if (list == null)
                return;
            if (list1 == null)
                return;
            time = (Environment.TickCount - TickStart) / 1000.0;
            list.Add(time, intsetpoint);
            list1.Add(time, intcurrent);
            Scale xSclae = zedGraphControl1.GraphPane.XAxis.Scale;
            if (time > xSclae.Max - xSclae.MajorStep)
            {
                if (intMode == 1)
                {
                    xSclae.Max = time + xSclae.MajorStep;
                    xSclae.Min = xSclae.Max - 30.0;
                }
                else
                {
                    xSclae.Max = time + xSclae.MajorStep;
                    xSclae.Min = 0;
                }

            }
            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }

        private void button1_Click_2(object sender, EventArgs e)
        {
            zedGraphControl1.GraphPane.CurveList.Clear();
            zedGraphControl1.GraphPane.GraphObjList.Clear();
            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
            graph_initiate();
            time = 0;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (PbMode.Text == "SROLL")
            {
                intMode = 1;
                PbMode.Text = "COMPACT";
            }
            else
            {
                intMode = 0;
                PbMode.Text = "SROLL";
            }
        }


        private void tbTest_TextChanged(object sender, EventArgs e)
        {
            byte_number = serialPort1.BytesToRead;
                data_in = new byte[byte_number];
        }

        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            this.Invoke(new EventHandler(ShowData));
        }

        private void ShowData(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen == true)
            {
                byte_number = serialPort1.BytesToRead;
                data_in = new byte[byte_number];
                if (byte_number != 0)
                {
                    textBox1.Text = byte_number.ToString();
                    serialPort1.Read(data_in, 0, byte_number);

                    if (byte_number == 13 && data_in[0] == 49 && data_in[12] == 57) //check byte_number, stx, etx
                    {
                        if (data_in[10] == 48)                      //          encoder data indication.
                        {
                            b++;
                            for (i = 0; i < 4; i++)
                            {
                                temp[i] = data_in[8 - i * 2];
                            }
                            data = System.BitConverter.ToInt32(temp, 0);
                            pos = data;
                            tbData.Text = data.ToString();

                            /*KIEM TRA GIA TRI ENCODER NHAN DUOC*
                            if (b == 10)
                            {
                                if ((data > (500 + pre_data)) || (data < (pre_data - 500)))
                                    data = pre_data;
                            }
                            if (b == 10)
                                b = 9;
                            /*----------------------*/

                            /*      DATA PROCESSING      */
                            if (data_out[4] == '1')                     //          encoder mode x4.
                            {     
                                i = data / (4 * ppr);
                                if (data > (ppr * 4) || data < (-ppr * 4))
                                    pos = (data - (i) * (ppr * 4));
                                pos = pos * 360 / (ppr*4);
                                w = (data - pre_data) * 1200 / (ppr * 4);
                            }
                            else if(data_out[4] == '0')                  //          encoder mode x1.
                            {
                                i = data / ppr;
                                if ((data > ppr) || (data < -ppr))
                                    pos = (data - (i) * ppr);
                                pos = pos * 360 / ppr;
                                w = (data - pre_data) * 1200 / ppr;
                            }
                            if (data_out[2] == '0')                     //          position controling.
                            {
                                draw(pos.ToString(), setpoint.ToString());
                            }
                            else if(data_out[2] == '1')                 //          speed controling.
                            {
                                draw(w.ToString(), setpoint.ToString());
                            }
                            pre_data = data;
                            /*----------------*/
                        }
                        else                                            //          if not enconder data, must be echo data.
                        {
                            /*      ECHO DATA CHECKING      */
                            for (i = 2; i < 9; i += 2)
                            {
                                lbltimerout.Text = "Timer_Out = No";
                                TimerOut.Enabled = false;
                                if (data_in[i] != data_out[i])
                                {
                                    serialPort1.Write(data_out, 0, 11); // resend control message if byte is not the same.
                                    TimerOut.Enabled = true;
                                }
                            }

                            tbTest.Text = data_in[0].ToString() + data_in[1].ToString() + data_in[2].ToString()
                             + data_in[3].ToString() + data_in[4].ToString() + data_in[5].ToString()
                             + (data_in[6]).ToString() + data_in[7].ToString() + (data_in[8]).ToString()
                             + data_in[9].ToString() + data_in[10].ToString() + data_in[11].ToString() + data_in[12].ToString();
                        }
                    }

                    else
                    {
                        serialPort1.Write(data_out, 0, 11);                    //   if the legth of received data's not equal to 13 --> resend control message.
                        TimerOut.Enabled = true;
                    }

                }
            }
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                data_out[6] = 1;
                data_out[8] = 144;
                serialPort1.Write(data_out, 0, 11);
                TimerOut.Enabled = true;
            }
        }

        private void time_out(object sender, EventArgs e)
        {
            serialPort1.Write(data_out, 0, 11);     //      resend data
            lbltimerout.Text = "Timer_Out = Yes";
        }

    }
}
