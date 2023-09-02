#define outputA 21
#define outputB 17
#define EncoderButton 22

int aState;
int aLastState;
unsigned long lastDebounceTime;
unsigned long StartFrameTime;
unsigned long StartPauseTime;
int PrevButtonState;
bool StateChanged;
Shutter ShutterStateTemp;

const unsigned long SettingChangeThd = 50;
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
    ShutterStateTemp = Stop;

    lastDebounceTime = 0;
    StartFrameTime = 0;
    StartPauseTime = 0;
    PrevButtonState = LOW;
    StateChanged = false;

    pinMode(outputA, INPUT);
    pinMode(outputB, INPUT);
    pinMode(EncoderButton, INPUT_PULLUP);
    aLastState = digitalRead(outputA);
}

void EncStateMachine()
{
    int ButtonState;
    
    ButtonState = digitalRead(EncoderButton);
    if (ButtonState != PrevButtonState)
    {
        lastDebounceTime = millis();
    }

    // button was released, so State Machine might change it's state again
    if (StateChanged == true)
    {
        if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == HIGH))
        {
            StateChanged = false;
        }
    }

    if (ShutterState == Stop)
    {
        switch (SettingSelected)
        {
            case Frames:
                FrameCntr = UpdateEncoder(FrameCntr);
                FrameCntr = RangeLimiter(FrameCntr, 1, 250);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    SettingSelected = Duration;
                    Serial.println("New state: Duration");
                    StateChanged = true;        //state changed, prevent from next changes
                }

                if (FrameCntr != FrameCntrPrev)
                {
                    Serial.print("Frame number: ");
                    Serial.println(FrameCntr);
                }

                FrameCntrPrev = FrameCntr;
                break;

            case Duration:
                FrameTime = UpdateEncoder(FrameTime);
                FrameTime = RangeLimiter(FrameTime, 1, 600);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    SettingSelected = Pause;
                    Serial.println("New state: Pause");
                    StateChanged = true;        //state changed, prevent from next changes
                }

                if (FrameTime != FrameTimePrev)
                {
                    Serial.print("Frame number: ");
                    Serial.println(FrameTime);
                }

                FrameTimePrev = FrameTime;
                break;

            case Pause:
                FramePause = UpdateEncoder(FramePause);
                FramePause = RangeLimiter(FramePause, 1, 60);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    SettingSelected = Execute;
                    Serial.println("New state: Execute");
                    StateChanged = true;        //state changed, prevent from next changes
                }

                if (FramePause != FramePausePrev)
                {
                    Serial.print("Frame pause: ");
                    Serial.println(FramePause);
                }

                FramePausePrev = FramePause;
                break;

            case Execute:
                ShutterStateTemp = UpdateEncoderShutter(ShutterStateTemp);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    if (ShutterStateTemp == Running)
                    {
                        ShutterState = Running;
                    }
                    else
                    {
                        SettingSelected = Frames;
                        Serial.println("New state: Frames");
                        StateChanged = true;        //state changed, prevent from next changes
                    }
                }

                if (ShutterStateTemp != ShutterStatePrev)
                {
                    Serial.print("State: ");
                    Serial.println(ShutterStateTemp);
                }

                ShutterStatePrev = ShutterStateTemp;
                break;

            default:
                break;
        }
    }
    else
    {
        if (((millis() - lastDebounceTime) > StateChangeThd) && (ButtonState == LOW) && (StateChanged == false))
        {
            ShutterState = Stop;
            ShutterStatePrev = Stop;
            ShutterStateTemp = Stop;
            SettingSelected = Frames;
            CurrFrame = 1;
            StateChanged = true;
            Serial.println("Return to settings: Frames");
        }
    }

    PrevButtonState = ButtonState;
}

Shutter UpdateEncoderShutter(Shutter counterUpdate)
{
    aState = digitalRead(outputA);
    if (aState != aLastState)
    {
        if (counterUpdate == Stop)
        { 
            counterUpdate = Running;
        }
        else
        {
            counterUpdate = Stop;
        }
    } 

    aLastState = aState; // Updates the previous state of the outputA with the current state
    return counterUpdate;
}

int UpdateEncoder(int counterUpdate)
{
    aState = digitalRead(outputA);
    if (aState != aLastState)
    {
        if (digitalRead(outputB) != aState)
        { 
            counterUpdate++;
        }
        else
        {
            counterUpdate--;
        }
    } 

    aLastState = aState; // Updates the previous state of the outputA with the current state
    return counterUpdate;
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