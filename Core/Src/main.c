/* USER CODE BEGIN Header */
#include "main.h"

TIM_HandleTypeDef htim4;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);

/* USER CODE BEGIN PV */
// ================== ???? (???? MIDI ??) ==================
#define REST 0
#define Bb5  932   // ?B5
#define C6   1047  // C6 (??Do)
#define D6   1175  // D6
#define D_SHARP_6 1245 // ?D6 / ?E6
#define E6   1319  // E6
#define F6   1397  // F6
#define G6   1568  // G6
#define A6   1760  // A6

// ????
#define OCTAVE_SHIFT   0      
#define GAP_SHORT      10     // ????? 10ms ??
#define LONG_THRESHOLD 600    // >=600ms ???,?????

// ? ????? ?
// 100 = ??,?????????130 ????????,??????
#define TEMPO_PERCENT  130    

const uint16_t led_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
                             GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

// ================== ?? (Lacie) - MIDI????? ==================
// 1-16?????,17-24?????????
const uint16_t melody_song[] = {
    // ?1-4?? (??)
    F6, E6, D6, E6, F6, G6,
    E6, E6, C6,
    D6, E6, D6, C6,
    C6,
    // ?5-8?? (???)
    F6, E6, D6, E6, F6, G6,
    E6, E6, C6,
    D6, F6, G6,
    A6, G6,
    // ?9-12??
    F6, E6, D6, E6, F6, G6,
    E6, E6, C6,
    D6, E6, D6, C6,
    C6, C6,
    // ?13-16?? (??)
    D6, E6, F6,
    E6, C6,
    D6,
    D6,

    // ===== ?17-24?? (?????? Everytime you kissed me) =====
    D_SHARP_6, D6, C6, D6, D_SHARP_6, F6,
    D6, D6, Bb5,
    C6, D6, C6, Bb5,
    Bb5, REST, REST,  // ???
    D_SHARP_6, D6, C6, D6, D_SHARP_6, F6,
    D6, D6, Bb5,
    C6, D_SHARP_6, F6,
    G6, F6
};

// ??????(ms) - ?? 120 BPM ?????
const uint16_t dur_song[] = {
    // 1-4 ??
    190, 190, 190, 190, 190, 190,
    400, 400, 400,
    190, 190, 400, 400,
    1350,
    // 5-8 ??
    190, 190, 190, 190, 190, 190,
    400, 400, 400,
    400, 400, 400,
    850, 400,
    // 9-12 ??
    190, 190, 190, 190, 190, 190,
    400, 400, 400,
    190, 190, 400, 400,
    750, 400,
    // 13-16 ??
    400, 400, 400,
    750, 400,
    1500,
    900,

    // 17-24 ?? (??)
    190, 190, 190, 190, 190, 190,
    400, 400, 400,
    190, 190, 400, 400,
    1000, 500, 500,
    190, 190, 190, 190, 190, 190,
    400, 400, 400,
    400, 400, 400,
    750, 400
};

// ????(????)
const uint16_t melody_scale[] = { C6, D6, E6, F6, G6, A6, Bb5, D_SHARP_6 };
const uint16_t dur_scale[]    = { 300, 300, 300, 300, 300, 300, 300, 600 };

// ================== ???? ==================
uint8_t  is_playing   = 0;
uint8_t  current_step = 1;
uint8_t  note_index   = 0;
uint8_t  phase        = 0;
uint32_t last_tick    = 0;

const uint16_t *current_melody;
const uint16_t *current_dur;
uint32_t        current_length;

uint32_t g_arr = 1000;

// ================== ????? ==================
void Buzzer_Set_Freq(uint32_t freq)
{
    if (freq == REST) {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
        return;
    }
    g_arr = 1000000 / freq - 1;
    if (g_arr < 2) g_arr = 2;

    __HAL_TIM_SET_AUTORELOAD(&htim4, g_arr);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (g_arr * 50) / 100);
}

void Buzzer_Set_Duty(uint8_t percent)
{
    if (percent > 90) percent = 90;
    if (percent < 1)  percent = 1;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (g_arr * percent) / 100);
}

void Buzzer_Mute(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
}

// ================== LED ==================
void Light_LED_By_Freq(uint32_t freq)
{
    HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);
    if (freq == REST) return;

    uint16_t notes[] = {Bb5, C6, D6, D_SHARP_6, E6, F6, G6, A6};
    for (int i = 0; i < 8; i++) {
        if (freq == notes[i]) {
            HAL_GPIO_WritePin(GPIOA, led_pins[i], GPIO_PIN_SET);
            return;
        }
    }
}

void Stop_Play(void)
{
    is_playing = 0;
    Buzzer_Mute();
    HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);
}
/* USER CODE END PV */


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM4_Init();

  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  Buzzer_Mute();

  uint8_t  key_state     = 1;
  uint8_t  key_processed = 0;
  uint32_t key_tick      = 0;
  /* USER CODE END 2 */

  while (1)
  {
      // ============== 1. ???? ==============
      uint8_t key_cur = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
      if (key_cur != key_state) {
          key_tick  = HAL_GetTick();
          key_state = key_cur;
      }

      if ((HAL_GetTick() - key_tick) > 20) {
          if (key_state == 0 && key_processed == 0) {
              key_processed = 1;

              if (current_step == 1) {
                  is_playing     = 1;
                  current_melody = melody_song;
                  current_dur    = dur_song;
                  current_length = sizeof(melody_song) / sizeof(melody_song[0]);
                  note_index = 0; phase = 0; last_tick = 0;
              }
              else if (current_step == 2) Stop_Play();
              else if (current_step == 3) {
                  is_playing     = 1;
                  current_melody = melody_scale;
                  current_dur    = dur_scale;
                  current_length = sizeof(melody_scale) / sizeof(melody_scale[0]);
                  note_index = 0; phase = 0; last_tick = 0;
              }
              else if (current_step == 4) Stop_Play();

              current_step++;
              if (current_step > 4) current_step = 1;
          }
          if (key_state == 1 && key_processed == 1) key_processed = 0;
      }

      // ============== 2. ???? ==============
      if (is_playing) {
          uint32_t now      = HAL_GetTick();
          
          // ?????????,??????????
          uint32_t duration = (current_dur[note_index] * TEMPO_PERCENT) / 100;
          uint32_t note     = current_melody[note_index];

          uint32_t gate;
          if (note == REST) {
              gate = 0;
          } else if (duration >= LONG_THRESHOLD) {
              gate = duration;               // ???????
          } else {
              gate = duration - GAP_SHORT;   // ???????
          }

          if (phase == 0) {
              if (last_tick == 0) {
                  if (note == REST) {
                      Buzzer_Mute();
                      HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);
                  } else {
                      Buzzer_Set_Freq(note);
                      Light_LED_By_Freq(note);
                  }
                  last_tick = now;
              }

              // ?????:???(50%) -> ????(35%) -> ????(28%)
              if (note != REST) {
                  uint32_t e = now - last_tick;
                  if (e < 15) Buzzer_Set_Duty(50);
                  else if (e < 80) Buzzer_Set_Duty(35);
                  else Buzzer_Set_Duty(28); 
              }

              if (now - last_tick >= gate) {
                  Buzzer_Mute();
                  phase     = 1;
                  last_tick = now;
              }
          }
          else {
              uint32_t rest_time = duration - gate;
              if (now - last_tick >= rest_time) {
                  HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);
                  note_index++;
                  if (note_index >= current_length) note_index = 0;
                  phase     = 0;
                  last_tick = 0;
              }
          }
      }
  }
}

/* --- ???????? --- */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) { Error_Handler(); }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) { Error_Handler(); }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) { Error_Handler(); }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) { Error_Handler(); }
  HAL_TIM_MspPostInit(&htim4);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
                          GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
                          GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void) { __disable_irq(); while (1) { } }