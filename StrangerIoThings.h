const char* ssid = "";
const char* password = "";


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>HTML Form to Input Data</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FF0000;}
  </style>
  </head><body>
  <h2>Text to display</h2>
  <form action="/get">
    Spell: <input type="text" name="input_string">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    Spell rapid: <input type="text" name="input_string">
    <input type="hidden" name="rapid">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get?yes_button=1">
    <input type="hidden" name="yes_button">
    <input type="submit" value="Yes">
  </form><br>
  <form action="/get?no_button=1">
  <input type="hidden" name="no_button">
    <input type="submit" value="No">
  </form><br>
    <form action="/get?stop=1">
    <input type="hidden" name="stop">
    <input type="submit" value="stop">
  </form><br>
    <form action="/get?run=1">
    <input type="hidden" name="run">
    <input type="submit" value="run">
  </form><br>
</body></html>)rawliteral";
