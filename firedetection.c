

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

static float mlx90640To[768]; // 
paramsMLX90640 mlx90640;

//function prototypes
boolean isConnected();

void setup()
{
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz
  pinMode(14, OUTPUT);
  digitalWrite(14,LOW);
  Serial.begin(115200);
  //set refresh rate
  MLX90640_SetRefreshRate(MLX90640_address, 0x04); 
  while (!Serial); //Wait for user to open terminal
  Serial.println("MLX90640 IR Array Example");

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (0);
  }
  Serial.println("MLX90640 online!");

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

  //Once params are extracted, we can release eeMLX90640 array
}

void loop()
{
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    //float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }
  
  boolean b2=true;
  int xq=0;
  int yq=0;
  for (int x = 0 ; x < 24 ; x++)
  {
    for(int y=31;y>=0;y--){
     
      if((int)(mlx90640To[x*32+y])>32.0){
        Serial.print("**");
        b2=false;
        xq=x;
        yq=y;
      }
      else{
        Serial.print((int)mlx90640To[x*32+y]);
        
      }
      Serial.print(" ");
      
    }
    Serial.println();
  }
  int counter=0;
  for(int x=0;x<768;x++){
    if((int)(mlx90640To[x])>25){
      counter++;
    }
    if((int)(mlx90640To[x])>28){
      counter+=2;
    }
    if((int)(mlx90640To[x])>32){
      counter++;
    }
  }

  counter=0;
  if(b2){
    for(int i=0;i<10;i++){
      Serial.println();
      delay(0);
    }
  }
  else{
    for(int i=0;i<5;i++){
      Serial.print("FIRE DETECTED AT COORDINATE: (");
      Serial.print(xq);
      Serial.print(",");
      Serial.print(yq);
      Serial.println(")!!!!");
      Serial.println((int)mlx90640To[xq*32+yq]);
      
    }
    
      digitalWrite(14,HIGH);
      delay(5000);
      digitalWrite(14,LOW);
    
    
  }
  
}

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}
