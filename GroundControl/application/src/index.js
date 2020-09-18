import React from 'react';
import ReactDOM from 'react-dom';
import App from './Views/App/App';
import MQTTProvider from './Hooks/MQTTProvider';
import './index.css';
import './constants.css';

const config = {
  host: '192.168.0.16',
  port: 1883
}

ReactDOM.render(
    <MQTTProvider config={config}>
      <App/>
    </MQTTProvider>,
  document.getElementById('root')
);
