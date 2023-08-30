#define outputA 21
#define outputB 17
#define EncoderButton 22
 
int counter = 0; 
int aState;
int aLastState;
unsigned long lastDebounceTime;
int PrevButtonState;

const unsigned long SettingChangeThd = 100;
const unsigned long StateChangeThd = 1000;
 
void InitEncoder()
{
    FrameCntr = 1;
    CurrFrame = 1;
    FrameTime = 1;
    FramePause = 1;
    ShutterState = Stop;
    SettingSelected = Frames;

    FrameCntrPrev = 1;
    CurrFramePrev = 1;
    FrameTimePrev = 1;
    FramePausePrev = 1;
    ShutterStatePrev = Stop;

    lastDebounceTime = 0;
    PrevButtonState = LOW;

    pinMode(outputA, INPUT);
    pinMode(outputB, INPUT);
    aLastState = digitalRead(outputA);
}

void EncStateMachine()
{
    int ButtonState = digitalRead(EncoderButton);
    if (ButtonState != PrevButtonState)
    {
        lastDebounceTime = millis();
    }

    if (ShutterState == Stop)
    {
        switch (SettingSelected)
        {
            case Frames:
                UpdateEncoder(&FrameCntr);
                FrameCntr = RangeLimiter(FrameCntr, 1, 250);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == HIGH))
                {
                    SettingSelected = Duration;
                }

                if (FrameCntr != FrameCntrPrev)
                {
                    Serial.print("Frame number: ");
                    Serial.println(FrameCntr);
                }

                FrameCntrPrev = FrameCntr;
                break;

            case Duration:
                UpdateEncoder(&FrameTime);
                FrameTime = RangeLimiter(FrameTime, 1, 600);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == HIGH))
                {
                    SettingSelected = Pause;
                }

                if (FrameTime != FrameTimePrev)
                {
                    Serial.print("Frame number: ");
                    Serial.println(FrameTime);
                }

                FrameTimePrev = FrameTime;
                break;

            case Pause:
                UpdateEncoder(&FramePause);
                FramePause = RangeLimiter(FramePause, 1, 60);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == HIGH))
                {
                    SettingSelected = Execute;
                }

                if (FramePause != FramePausePrev)
                {
                    Serial.print("Frame pause: ");
                    Serial.println(FramePause);
                }

                FramePausePrev = FramePause;
                break;

            case Execute:
                UpdateEncoderShutter(&ShutterState);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == HIGH))
                {
                    SettingSelected = Frames;
                }

                if (ShutterState != ShutterStatePrev)
                {
                    Serial.print("State: ");
                    Serial.println(ShutterState);
                }

                ShutterStatePrev = ShutterState;
                break;

            default:
                break;
        }
    }
    else
    {
        if (((millis() - lastDebounceTime) > StateChangeThd) && (ButtonState == HIGH))
        {
            ShutterState = Stop;
        }
    }
}

void UpdateEncoderShutter(Shutter *counterUpdate)
{
    aState = digitalRead(outputA);
    if (aState != aLastState)
    {
        if (*counterUpdate == Stop)
        { 
            *counterUpdate = Running;
        }
        else
        {
            *counterUpdate = Stop;
        }
    } 

    aLastState = aState; // Updates the previous state of the outputA with the current state
}

void UpdateEncoder(int *counterUpdate)
{
    aState = digitalRead(outputA);
    if (aState != aLastState)
    {
        if (digitalRead(outputB) != aState)
        { 
            *counterUpdate++;
        }
        else
        {
            *counterUpdate--;
        }
    } 

    aLastState = aState; // Updates the previous state of the outputA with the current state
}

int RangeLimiter (int input, int minValue, int maxValue)
{
    if (input > maxValue)
    {
        return maxValue;
    }
    else if (input < minValue)
    {
        return minValue;
    }
    else
    {
        return input;
    }
}