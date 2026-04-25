import 'dotenv/config';
import { App, LogLevel } from '@slack/bolt';
import OpenAI from 'openai';

const requiredEnv = ['SLACK_BOT_TOKEN', 'SLACK_APP_TOKEN', 'OPENAI_API_KEY'];
const missingEnv = requiredEnv.filter((name) => !process.env[name]);

if (missingEnv.length > 0) {
  throw new Error(`Missing required environment variables: ${missingEnv.join(', ')}`);
}

const app = new App({
  token: process.env.SLACK_BOT_TOKEN,
  appToken: process.env.SLACK_APP_TOKEN,
  socketMode: true,
  logLevel: process.env.SLACK_LOG_LEVEL || LogLevel.INFO,
});

const openai = new OpenAI({
  apiKey: process.env.OPENAI_API_KEY,
  baseURL: process.env.OPENAI_BASE_URL || undefined,
});

const model = process.env.OPENAI_MODEL || 'gpt-4.1-mini';

const systemPrompt = `
You are a concise Hungarian-speaking coding/product assistant available from Slack.
Answer in Hungarian by default. Keep replies practical and short unless the user asks for detail.
If the user asks you to change code or perform repository actions, explain that this Slack bot can advise,
but actual repo edits must be run by a Cursor agent session unless a separate execution backend is connected.
`.trim();

function stripBotMention(text) {
  return text.replace(/<@[A-Z0-9]+>/g, '').trim();
}

async function generateReply(userText) {
  const response = await openai.responses.create({
    model,
    input: [
      {
        role: 'system',
        content: systemPrompt,
      },
      {
        role: 'user',
        content: userText,
      },
    ],
  });

  return response.output_text?.trim() || 'Nem kaptam értelmezhető választ a modelltől.';
}

async function handleMessage({ event, say, client }) {
  if (event.bot_id || event.subtype === 'bot_message') {
    return;
  }

  const text = stripBotMention(event.text || '');

  if (!text) {
    await say({
      text: 'Itt vagyok. Írd le, mit szeretnél.',
      thread_ts: event.thread_ts || event.ts,
    });
    return;
  }

  await client.reactions.add({
    channel: event.channel,
    timestamp: event.ts,
    name: 'eyes',
  }).catch(() => {});

  try {
    const reply = await generateReply(text);
    await say({
      text: reply,
      thread_ts: event.thread_ts || event.ts,
    });
  } catch (error) {
    console.error(error);
    await say({
      text: 'Hiba történt válaszadás közben. Nézd meg a szerver logot.',
      thread_ts: event.thread_ts || event.ts,
    });
  }
}

app.event('app_mention', handleMessage);
app.message(async (args) => {
  if (!args.event.subtype && args.event.channel_type === 'im') {
    await handleMessage(args);
  }
});

await app.start();
console.log('Slack agent is running in Socket Mode.');
