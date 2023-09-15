#define outputA 21
#define outputB 17
#define EncoderButton 22
#define MAXFRAMECNTR 500
#define MAXFRAMETIME 600
#define MAXPAUSETIME 60
#define EXPOSUREADD 100

int FrameCntrPrev;
int CurrFramePrev;
int FrameTimePrev;
int FramePausePrev;

int aState;
int aLastState;
unsigned long lastDebounceTime;
int PrevButtonState;
bool StateChanged;
bool SequenceFinished;

unsigned long StartFrameTime;
unsigned long StartPauseTime;

const unsigned long SettingChangeThd = 50;
const unsigned long StateChangeThd = 1000;
 
void InitEncoder()
{
    FrameCntr = 1;
    CurrFrame = 1;
    FrameTime = 1;
    CurrTime = 1;
    FramePause = 1;
    PauseTime = 1;
    ShutterState = Stop;
    SettingSelected = Frames;

    FrameCntrPrev = 1;
    CurrFramePrev = 1;
    FrameTimePrev = 1;
    FramePausePrev = 1;
    ShutterStatePrev = Stop;
    ShutterStateInt = Stop;

    lastDebounceTime = 0;
    PrevButtonState = LOW;
    StateChanged = false;
    SequenceFinished = false;

    StartFrameTime = 0;
    StartPauseTime = 0;
    ExposureActive = false;
    PauseActive = false;

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
                FrameCntr = RangeLimiter(FrameCntr, 1, MAXFRAMECNTR);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    SettingSelected = Duration;
                    // Serial.println("New state: Duration");
                    StateChanged = true;        //state changed, prevent from next changes
                }

                // if (FrameCntr != FrameCntrPrev)
                // {
                //     Serial.print("Frame number: ");
                //     Serial.println(FrameCntr);
                // }

                FrameCntrPrev = FrameCntr;
                break;

            case Duration:
                FrameTime = UpdateEncoder(FrameTime);
                FrameTime = RangeLimiter(FrameTime, 1, MAXFRAMETIME);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    SettingSelected = Pause;
                    // Serial.println("New state: Pause");
                    StateChanged = true;        //state changed, prevent from next changes
                }

                // if (FrameTime != FrameTimePrev)
                // {
                //     Serial.print("Frame number: ");
                //     Serial.println(FrameTime);
                // }

                FrameTimePrev = FrameTime;
                break;

            case Pause:
                FramePause = UpdateEncoder(FramePause);
                FramePause = RangeLimiter(FramePause, 1, MAXPAUSETIME);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    SettingSelected = Execute;
                    // Serial.println("New state: Execute");
                    StateChanged = true;        //state changed, prevent from next changes
                }

                // if (FramePause != FramePausePrev)
                // {
                //     Serial.print("Frame pause: ");
                //     Serial.println(FramePause);
                // }

                FramePausePrev = FramePause;
                break;

            case Execute:
                ShutterStateInt = UpdateEncoderShutter(ShutterStateInt);

                if (((millis() - lastDebounceTime) > SettingChangeThd) && (ButtonState == LOW) && (StateChanged == false))
                {
                    if (ShutterStateInt == Running)
                    {
                        ShutterState = Running;
                    }
                    else
                    {
                        SettingSelected = Frames;
                        // Serial.println("New state: Frames");
                        StateChanged = true;        //state changed, prevent from next changes
                    }
                }

                // if (ShutterStateInt != ShutterStatePrev)
                // {
                //     Serial.print("State: ");
                //     Serial.println(ShutterStateInt);
                // }

                ShutterStatePrev = ShutterStateInt;
                break;

            default:
                break;
        }
    }
    else
    {
        if ((((millis() - lastDebounceTime) > StateChangeThd) && (ButtonState == LOW) && (StateChanged == false)) || (SequenceFinished == true))
        {
            if (ExposureActive == true)
            {
                canon_ble.trigger();
            }
            ShutterState = Stop;
            ShutterStatePrev = Stop;
            ShutterStateInt = Stop;
            SettingSelected = Frames;
            CurrFrame = 1;
            CurrTime = 1;
            PauseTime = 1;
            StateChanged = true;
            ExposureActive = false;
            PauseActive = false;
            SequenceFinished = false;
            // Serial.println("Return to settings: Frames");
        }
        else
        {
            if ((ExposureActive == false) && (PauseActive == false))
            {
                Serial.println("Start exposure ");
                // Serial.println(CurrFrame);
                ExposureActive = true;
                StartFrameTime = millis();
                canon_ble.trigger();
                /*Begin exposure*/
            }
            else if (PauseActive == false)
            {
                CurrTime = (int)(millis() - StartFrameTime)/1000;
                if ((millis() - StartFrameTime) > ((FrameTime * 1000) + EXPOSUREADD))
                {
                    Serial.println("Start pause ");
                    // Serial.println(FramePause);
                    canon_ble.trigger();
                    ExposureActive = false;
                    PauseActive = true;
                    StartPauseTime = millis();
                    CurrTime = 1;
                    /*End exposure*/
                }
                else
                {
                    //Do nothing, exposure in progress
                }
            }
            else
            {
                PauseTime = (int)(millis() - StartPauseTime)/1000;
                if ((millis() - StartPauseTime) > (FramePause * 1000))
                {
                    // Serial.println("Stop pause");
                    PauseActive = false;
                    PauseTime = 1;
                    CurrFrame++;
                }
                else
                {
                    //Do nothing, pause in progress
                }
            }

            if (CurrFrame > FrameCntr)
            {
                SequenceFinished = true;
            }
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