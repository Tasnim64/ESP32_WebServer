String HTML_PAGE = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="refresh" content="300"> <!-- Refresh the page every 5 minutes -->
    <title>ESP32 Sonar Server</title>
    <style>
        /* Your CSS styles here */
        body {
            font-family: Courier New, monospace;
            background-color: #f2f2f2;
        }

        @keyframes colorChange {
            /* CSS animation for background color change */
            0% {
                background-color: #5F2B8F;
                border-color: #97329B;
            }
            50% {
                background-color: #009498;
                border-color: #009498;
            }
            100% {
                background-color: #97329B;
                border-color: #28CDD2;
            }
        }

        header {
            /* Styling for the header section */
            background-color: #5D0AAB;
            color: #fff;
            text-align: center;
            padding: 3rem;
            animation: colorChange 10s infinite alternate; /* Apply background color animation */
            border: 6px solid #28CDD2;
        }

        main {
            /* Styling for the main content */
            padding: 6rem;
            text-align: center;
        }

        .color-bar-container {
            /* Container for the color-changing bar */
            width: 100%;
            height: 20px;
            position: relative;
        }

        .color-bar {
            /* Color-changing bar inside the container */
            height: 100%;
            position: absolute;
            top: 0;
            left: 0;
            transition: width 0.5s ease-in-out; /* Animate width changes */
            width: 0%; /* Initially set to 0 width */
        }

        .distance-display {
            /* Styling for the distance display section */
            font-size: 15px;
            margin-top: 60px;
            position: relative;
        }

        .neon-sign {
            /* Neon sign styling for alerts */
            font-size: 24px;
            color: #FF0000; /* Red color */
            text-align: center;
            margin-top: 50px;
            text-transform: uppercase;
            font-weight: bold;
            animation: neonBlink 1s infinite alternate; /* Apply neon blinking animation */
            position: absolute;
            left: 50%;
            transform: translateX(-50%);
            display: none; /* Initially hidden */
        }

        @keyframes neonBlink {
            /* CSS animation for neon blinking effect */
            from {
                opacity: 1;
            }
            to {
                opacity: 0.5;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>ESP32 Sonar Server</h1>
        <p>Real-time Sonar Data</p>
    </header>
    <main>
        <!-- Color-changing bar and distance display -->
        <div class="color-bar-container">
            <div class="color-bar" style="background-color: #6614B4;"></div>
        </div>
        <div class="distance-display">
            Distance: 
            <span id="distanceCm">0.00 cm</span>  |
            <span id="distanceInch">0.00 inch</span> |
            <span id="distanceMeter">0.00 meter</span> 
            <div class="neon-sign">No Alert</div>
        </div>
    </main>

    <script>
        // Function to calculate the color and width of the color bar based on distance
        function calculateWidthAndColor(distanceCm) {
            // Define the total width in centimeters (400 cm)
            const totalWidthCm = 400.0;

            // Calculate the width percentage
            const widthPercentage = (distanceCm / totalWidthCm) * 100;

            // Determine the color based on the distance
            let color;
            if (distanceCm >= 0 && distanceCm <= 5) {
                color = 'red';
            } else if (distanceCm > 5 && distanceCm <= 10) {
                color = 'yellow';
            } else if (distanceCm > 10 && distanceCm <= 15) {
                color = 'green';
            } else if (distanceCm > 15 && distanceCm <= 400) {
                color = 'blue';
            }
            // Ensure the width percentage is between 0% and 100%
            const clampedWidthPercentage = Math.min(Math.max(widthPercentage, 0), 100);
            // Return both the width percentage and the color
            return { widthPercentage: clampedWidthPercentage, color };
        }

        // Function to get color based on width
        function getColorForWidth(width) {
            if (width >= 0 && width <= 1.25) {
                return '#D32408'; 
            } else if (width > 1.5 && width <= 2.50) {
                return '#DAC228';
            } else if (width > 2.50 && width <= 3.75) {
                return '#08A850'; 
            } else if (width > 3.75 && width <= 100) {
                return '#781798'; 
            } else {
                return '#781798'; 
            }
        }

        // Function to get alert message based on distance
        function getAlertMessage(distanceCm) {
            if (distanceCm <= 5) {
                return "ALERT ALERT ALERT";
            } else if (distanceCm <= 10) {
                return "ALERT ALERT";
            } else if (distanceCm <= 15) {
                return "ALERT";
            } else {
                return "No Alert";
            }
        }

        // Function to update data from the server
        function updateData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var data = JSON.parse(this.responseText);
                    console.log("Received data:", data);

                    // Update the distance values
                    document.getElementById('distanceCm').textContent = data.distance_cm.toFixed(2) + ' cm';
                    document.getElementById('distanceMeter').textContent = (data.distance_cm / 100).toFixed(2) + ' meter';
                    document.getElementById('distanceInch').textContent = (data.distance_cm / 2.54).toFixed(2) + ' inch';

                    // Calculate the color bar width based on the distance
                    var colorBarWidth = calculateWidthAndColor(data.distance_cm);

                    // Update the color bar width
                    var colorBar = document.querySelector('.color-bar');
                    console.log('Received color_bar_width:', data.color_bar_width);
                    colorBar.style.width = data.color_bar_width + '%';
                    colorBar.style.backgroundColor = getColorForWidth(colorBarWidth.widthPercentage);

                    // Check if there's an alert signal from ESP32 and display the neon sign accordingly
                    var alertMessage = getAlertMessage(data.distance_cm);
                    var neonSign = document.querySelector('.neon-sign');

                    if (alertMessage !== "No Alert") {
                        // Display the alert message and show the neon sign
                        neonSign.style.display = 'block';
                        neonSign.textContent = alertMessage;
                    } else {
                        // Hide the neon sign if no alert
                        neonSign.style.display = 'none';
                    }
                }
            };
            xhttp.open('GET', '/api', true); // Replace '/api' with the actual API endpoint
            xhttp.send();
        }

        // Initial data update
        updateData();

        // Set up periodic data refresh
        setInterval(updateData, 1000); // Refresh data every 1 second
    </script>
</body>
</html>
)=====";
