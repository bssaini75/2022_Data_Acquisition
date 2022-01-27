
//Data_Sampler_07: Receives acknowledgement from RPi

int threshold=1;

const int array_size=192;
const int read_ahead=2;
const int temp_size=6; //Always keep an even number
const int temp_half=(temp_size/2);

int i=0;
int avg_old;
int avg_new;
int time_taken;

float time_for_one_set = 0;
float DUE_ADC_Polling_time = 0; //in microseconds
float Time_A0 = 0;

int data_transmitted_A0[array_size+temp_size+read_ahead];
int temp_A0[temp_size];

int data_transmitted_A2[array_size+temp_size+read_ahead];
int temp_A2[temp_size];

int data_transmitted_A4[array_size+temp_size+read_ahead];
int temp_A4[temp_size];

int data_transmitted_A7[array_size+temp_size+read_ahead];
int temp_A7[temp_size];

long delay_time_us=4;

void setup() 
{
 Serial.begin(9600);
 Serial1.begin(9600); //baud rate 9600 for the Bluetooth communication
 //analogReadResolution(12);
 pinMode(A0,INPUT);
 pinMode(A2,INPUT);
 pinMode(A4,INPUT);
 pinMode(A7,INPUT);
}

void loop() 
{
  if (Serial1.available() > 0)
  {
    String data = Serial1.readStringUntil('\n');
    Serial.print("RPi sent: ");
    //Serial1.print("Arduino acknowledges: ");
    Serial.println(data);
    //Serial1.println(data);
    delay(1000);
  }
  avg_old=0;
  avg_new=0;

  for(i=0;i<read_ahead;i++) //Read ahead, before the impact
  {
    data_transmitted_A0[i]=analogRead(A0);
    data_transmitted_A2[i]=analogRead(A2);
    data_transmitted_A4[i]=analogRead(A4);
    data_transmitted_A7[i]=analogRead(A7);
    delayMicroseconds(delay_time_us);
  }
  for(i=0;i<temp_size;i++)
  {
   temp_A0[i]=analogRead(A0);
   temp_A2[i]=analogRead(A2);
   temp_A4[i]=analogRead(A4);
   temp_A7[i]=analogRead(A7);
   delayMicroseconds(delay_time_us);
  }
  //Serial.println("Loop Start");
  for(i=0;i<temp_half;i++) //Comparison of recorded values for detecting impact: Time of around 5 microsends for temp_size[] of 10 spent in this check is ignored
  {
   if(temp_A7[i]<2048)
   {
     avg_old+=((2048-temp_A0[i])+2048); //Rectification of -ve values
     //Serial.print("avg_old: ");
     //Serial.println(avg_old);
   }
   if(temp_A7[i]>=2048)
   {
     avg_old+=temp_A0[i];
     //Serial.print("avg_old: ");
     //Serial.println(avg_old);  
   }
  }
  for(i=temp_half;i<temp_size;i++)
  {
   if(temp_A7[i]<2048)
   {
     avg_new+=(2048-temp_A7[i])+2048; //Rectification of -ve values
     //Serial.print("avg_new: ");
     //Serial.println(avg_new);
   }

   if(temp_A7[i]>=2048)
   {
     avg_new+=temp_A7[i];  
     //Serial.print("avg_new: ");
     //Serial.println(avg_new);
   }
  }
  //Serial.print("Difference: ");
  //Serial.println(avg_new-avg_old);
  //delay(1000);
  if(avg_new-avg_old>threshold||avg_old-avg_new>threshold)
  {
    for(i=0;i<temp_size;i++) //First, copy the data into arrays; time spent is ignored
    {
      data_transmitted_A0[i+read_ahead]=temp_A0[i];
      data_transmitted_A2[i+read_ahead]=temp_A2[i];
      data_transmitted_A4[i+read_ahead]=temp_A4[i];
      data_transmitted_A7[i+read_ahead]=temp_A7[i];
    }
    time_taken=millis();
    for(i=temp_size+read_ahead;i<array_size+temp_size+read_ahead;i++) //Read the remaining elements from ADC channels
    {
      data_transmitted_A0[i]=analogRead(A0);
      data_transmitted_A2[i]=analogRead(A2);
      data_transmitted_A4[i]=analogRead(A4);
      data_transmitted_A7[i]=analogRead(A7);
      delayMicroseconds(delay_time_us);
    }
    time_taken=millis()-time_taken;
    time_for_one_set /*of 4 readings*/= time_taken*1000/(array_size); //in microseconds
    DUE_ADC_Polling_time = (time_for_one_set-delay_time_us)/4; //in microseconds
    
    for(i=0;i<array_size+temp_size+read_ahead;i++) //Normalize the recorded data
    {
      data_transmitted_A0[i]-=2048;
      data_transmitted_A2[i]-=2048;
      data_transmitted_A4[i]-=2048;
      data_transmitted_A7[i]-=2048;
    }
    
    //Transmit summary over USB
    Serial.print('\n');
    Serial.print("Detection threshold value:");
    Serial.println(threshold);
    Serial.print("Difference value:");
    Serial.println(avg_new-avg_old);
    Serial.print("Time between samples(us):");
    Serial.println(delay_time_us);
    Serial.print("Time taken for all samples(ms):");
    Serial.println(time_taken);

    //Print headers via USB
    Serial.print("Time_Interval"); //time_for_one_set
    Serial.print(",");
    Serial.print("Time_ADC0");
    Serial.print(",");
    Serial.print("ADC0");
    Serial.print(",");
    Serial.print("Time_ADC2");
    Serial.print(",");
    Serial.print("ADC2");
    Serial.print(",");
    Serial.print("Time_ADC4");
    Serial.print(",");
    Serial.print("ADC4");
    Serial.print(",");
    Serial.print("Time_ADC7");
    Serial.print(",");
    Serial.print("ADC7");
    Serial.print('\n');
    //Print headers via bluetooth
    Serial1.print("Time_Interval"); //time_for_one_set
    Serial1.print(",");
    Serial1.print("Time_ADC0");
    Serial1.print(",");
    Serial1.print("ADC0");
    Serial1.print(",");
    Serial1.print("Time_ADC2");
    Serial1.print(",");
    Serial1.print("ADC2");
    Serial1.print(",");
    Serial1.print("Time_ADC4");
    Serial1.print(",");
    Serial1.print("ADC4");
    Serial1.print(",");
    Serial1.print("Time_ADC7");
    Serial1.print(",");
    Serial1.print("ADC7");
    Serial1.print('\n');
    Time_A0=0;
    for(i=0;i<array_size+temp_size+read_ahead;i++)
    {
       //Transmission via Bluetooth and USB
       Serial.print(time_for_one_set); //time_for_one_set
       Serial.print(",");
       Serial.print(Time_A0);
       Serial.print(",");
       Serial.print(data_transmitted_A0[i]);
       Serial.print(",");
       Serial1.print(time_for_one_set); //time_for_one_set
       Serial1.print(",");
       Serial1.print(Time_A0);
       Serial1.print(",");
       Serial1.print(data_transmitted_A0[i]);
       Serial1.print(",");
       Time_A0=Time_A0+DUE_ADC_Polling_time;
       Serial.print(Time_A0);
       Serial.print(",");
       Serial.print(data_transmitted_A2[i]);
       Serial.print(",");
       Serial1.print(Time_A0);
       Serial1.print(",");
       Serial1.print(data_transmitted_A2[i]);
       Serial1.print(",");
       Time_A0=Time_A0+DUE_ADC_Polling_time;
       Serial.print(Time_A0);
       Serial.print(",");
       Serial.print(data_transmitted_A4[i]);
       Serial.print(",");
       Serial1.print(Time_A0);
       Serial1.print(",");
       Serial1.print(data_transmitted_A4[i]);
       Serial1.print(",");
       Time_A0=Time_A0+DUE_ADC_Polling_time;
       Serial.print(Time_A0);
       Serial.print(",");
       Serial.print(data_transmitted_A7[i]);
       Serial.print('\n');
       Serial1.print(Time_A0);
       Serial1.print(",");
       Serial1.print(data_transmitted_A7[i]);
       Serial1.print('\n');
       Time_A0 = Time_A0+DUE_ADC_Polling_time+delay_time_us;
    }
    Serial1.print('\n');
    Serial1.print("End");
    Serial1.print('\n');
  }
}
