const char paginaInicial[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <title>El-Canterito</title>
        <style>
            .switch {
                position: relative;
                display: inline-block;
                width: 60px;
                height: 34px;
            }

            .switch input {
                opacity: 0;
                width: 0;
                height: 0;
            }

            .slider {
                position: absolute;
                cursor: pointer;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background-color: #ccc;
                -webkit-transition: .4s;
                transition: .4s;
            }

            .slider:before {
                position: absolute;
                content: "";
                height: 26px;
                width: 26px;
                left: 4px;
                bottom: 4px;
                background-color: white;
                -webkit-transition: .4s;
                transition: .4s;
            }

            input:checked + .slider {
                background-color: #2196F3;
            }

            input:focus + .slider {
                box-shadow: 0 0 1px #2196F3;
            }

            input:checked + .slider:before {
                -webkit-transform: translateX(26px);
                -ms-transform: translateX(26px);
                transform: translateX(26px);
            }

            /* Rounded sliders */
            .slider.round {
                border-radius: 34px;
            }

            .slider.round:before {
                border-radius: 50%;
            }

            input{
                font-size: 2em;
                border-radius: 1em;
                width: 4em;
                border-width: 0;
            }
            body{
                background-color: #f9e79f;
            }
        </style>
    </head>
    <body>
        <h1>El-Canterito</h1>
        <section>
            <h2>Canteiro</h2>
            <label><span id="regando">Desligado</span></label><br/>
            <label class="switch">
                <input type="checkbox" name="ligarRega" onchange="toggleState(this.checked)">
                <span class="slider round"></span>
            </label>
            <br/>
            <label for="tempoMinimoDeRega">Tempo de rega em minutos</label><br/>
            <input type="number" name="tempoMinimoDeRega" id="tempoMinimoDeRega" value="15" onchange="setMinTime(this.value)">
            <br/>
            <label for="intervaloDeRega">Intervalo entre as regas</label><br/>
            <input type="number" name="intervaloDeRega" id="intervaloDeRega" value="12" onchange="setSchedule(this.value)">
        </section>
        <script>
            function toggleState(state)
            {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function () {
                    if (this.readyState === 4 && this.status === 200) {
                        document.getElementById("regando").innerHTML = this.responseText;
                    }
                };

                xhttp.open("GET", "ledToggle?state=" + state, true);
                xhttp.send();
            }

            function setSchedule(hours)
            {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function () {
                    if (this.readyState === 4 && this.status === 200) {
                        document.getElementById("intervaloDeRega").innerHTML = this.responseText;
                    }
                };

                xhttp.open("GET", "interval?hours=" + hours, true);
                xhttp.send();
            }

            function setMinTime(minutes)
            {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function () {
                    if (this.readyState === 4 && this.status === 200) {
                        document.getElementById("intervaloDeRega").innerHTML = this.responseText;
                    }
                };

                xhttp.open("GET", "mintime?minutes=" + minutes, true);
                xhttp.send();
            }

            function getData() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function () {
                    if (this.readyState === 4 && this.status === 200) {
                        document.getElementById("regando").innerHTML =
                                this.responseText;
                    }
                };
                xhttp.open("GET", "status", true);
                xhttp.send();
            }

            setInterval(function () {
                getData();
            }, 1000);
        </script>
    </body>
</html>
)=====";